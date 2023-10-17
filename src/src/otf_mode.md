# 800x600x64 On-the-Fly (OTF) Mode

The AgonLight VDP-GL (video display processor graphics library) is based on a fork of the FabGL, and includes support for VGA displays at various resolutions and color palettes, plus support for the keyboard, mouse, and serial communication btween the EZ80 CPU and the ESP32 CPU.

FabGL, and thus its VDP-GL derivative, processes graphics primitives (such as points, lines, triangles, and bitmaps) by writing to one or two full-frame memory buffers, and then allowing I2S DMA hardware to output the VGA pixel data by reading buffered bytes. The final output includes signals for 8 bits: 1 horizontal synchronization bit, 1 vertical synchronization bit, 2 red color bits, 2 green color bits, and 2 blue color bits. Because there are 6 color bits, the AgonLight can show a maximum of 64 distinct colors.

If two buffers are used, VDP-GL can write to one buffer while the hardware outputs the other buffer. If only one buffer is used, VDP-GL can only write to it during the vertical blanking period, to avoid visible tearing, a situation where parts of 2 successive frames are shown, rather than one complete frame.

Because DMA-accessible RAM, also known as internal RAM, in the ESP32 is limited, there is insufficient RAM available to display all 64 possible colors at resolutions higher than about one-quarter of VGA, meaning about 320x200 or 320x240 pixels.

To show more colors at a higher resolution is the goal of the 800x600x64 mode, dubbed the On-the-Fly (OTF) mode, because of how it works. This special mode conserves DMA RAM by using only 8 horizontal scan line buffers, as opposed to using 1 scan line buffer per vertical line of resolution. Thus, it does not need 600 scan line buffers for active (visible) pixels. Instead of drawing a complete video frame before outputting that frame, it draws individual scan lines on-the-fly while the frame is being output, making every attempt to stay slightly ahead of the DMA hardware, so as to avoid causing visible flickering.

The OTF mode runs using a pixel clock of 40 MHz in order to provide the required 800x600 visible pixels, along with the hidden horizontal and vertical blanking pixels, at a frame rate of 60 Hz
(60 FPS, meaning 60 visible frames per second). The pixel clock in this mode is significantly
faster than the 25.175 MHz clock used for VGA with 640x480 pixels at 60 FPS. Therefore, the amount
of time needed for each scan line in OTF mode is significantly less than the amount of time needed
for each scan line in 640x480 mode, or any mode using a slower pixel clock.

For that reason, all of the core drawing functions for OTF mode are written in ESP32 assembly language,
to make those functions as fast as possible. The drawing primitives are C++ classes, with common
and non-common data members and function members, but these members are all geared toward helping
the core assembler routines run very quickly. This means that various data elements, such as
pixel colors for bitmaps, and tile information for tile maps, are stored in specific formats that
will help to reduce decision-making while drawing the primitives. For example, if a bitmap supports
horizontal scrolling (on a 1-pixel boundary), there are multiple copies of the bitmap in memory,
each pre-shifted based on the pixel alignment. This allows the bitmap to be copied very quickly,
a whole word (4 bytes) at a time, when drawing, regardless of the destination byte alignment.

# OTF Strategy

In the OTF mode, each drawing primitive is a node in a tree of primitives. The tree branches (nesting) can
serve to affect how primitives are moved, drawn, and clipped. For example, in the simplest case, if primitve B
is a child of primitive A, and A is moved (in X and/or Y), then B is moved along with A, so that its relative
postion with respect to A does not change, but its physical position on the display does change. Thus, an entire array of enemy ships can
be moved simply by moving a single "group" primitive that is the parent of all of the ships.

Another important aspect of how the OTF mode works is that it must keep all primitives in internal RAM,
so that it can draw them repeatedly, without being told to do so by the BASIC (EZ80) application. Since
the ESP32 holds the primitives in RAM, they can easily be manipulated (e.g., moved), without sending
all of the information required to recreate them across the serial channel. VDP-GL already does something
similar with bitmaps and sprites, but as a Fab-GL derivative, not with simpler primitives such as lines and triangles. However,
enhancements are in the works that allow VDP-GL to create buffered
commands, so that it can replay them without a large data transfer.

When primitives are no longer needed by the BASIC application, they may be deleted. Deleting a group
primitive will delete its children, too, reducing the number of serial commands required to delete
the parent and all of its children.

One other benefit of keeping the primitives in RAM is the ability to turn them on and off, meaning
to show them or to hide them, very easily, just by changing their flag bits. For example, a blinking
cursor could be actuated just by changing its flag bit for being painted (drawn). The cursor itself can
be any primitive (a point, line, rectangle, etc.), based on the needs of the application.

# OTF Critical Section

There are certain aspects to how the OTF mode draws (paints) the screen that
should be kept in mind when designing the graphics for applications.

* <b>Timing is everything.</b> At a pixel clock rate of 40 MHz, each scan line,
including the visible and invisible portions of it, lasts just 0.0000264 seconds
(which is 26.4 microseconds). Not a lot of time! At the 240 MHz internal clock
rate of the ESP32, each clock tick lasts about 0.004167 microseconds (which is
4.167 picoseconds). Dividing the scan line time by the CPU clock tick time
indicates that there are approximately 6336 CPU clock cycles per scan line.
During those cycles, the CPU must execute enough instructions to draw one line.
While one scan line is being displayed by the I2S hardware, the OTF manager must
draw (create 800 colored pixels for) a future scan line, and it must do so in under
6336 CPU clock cycles. Again, not a lot of time.<br><br>For this reason, it is critical to avoid drawing (layering) too many things on the same scan line. Each time the same pixel in the current scan line buffer is written,
time is used, so if there are too many objects (primitives) layered on top of
each other, there may not be enough time to draw the scan line, before the next
scan line should be drawn. If time runs out, the screen will show flashing
(flickering) scan lines. It is unlikely that an entire object (such as a sprite)
will flicker on and off. What is much more likely is that distinct scan lines
within the object will flicker, or be vertically out of place.<br><br>Another reason that flickering may occur, meaning that the time to draw a scan line exceeds the limit, is that CPU time can be stolen away from the OTF manager
via interrupts and by other tasks. To help to reduce this possibility, the OTF manager runs in a high-priority task.

* <b>Everything must be painted.</b> Every scan line on the screen (all 600 of them) must be drawn (painted) on every frame (i.e., 60 times per second). What
happens if one of those scan lines is not painted? Whatever was left in the scan
line buffer will be displayed again, at whatever vertical position the I2S hardware happens to be outputting. To exaggerate a bit, if the code only painted the first (top) scan line (out of the 8 lines used) once, and never again, then the display would show that scan line's pixels on the screen 75 times, once per group of 8 lines, going down the screen's 600 lines.<br><br>
So, how does this requirement affect the EZ80 application, and how does it affect the OTF manager in the ESP32? Let's take the latter part first. In order
to redraw each active primitive 60 times per second, the manager must keep them
all in RAM (as mentioned above). Primitives are not transferred, written, and
deleted. They are stored for continual use, until the EZ80 application sends the
OTF commands to delete them. There is a big advantage to having them kept in RAM, because they can be used for a long time, without being transferred across
the serial port again (i.e., sending the command(s) to create a primitive once
is enough).<br><br>
Now, on to the former part. In order to make sure that every scan line is painted during each frame, the EZ80 application must define at least 1 primitive
that covers each pixel on the screen, so that there is at least one "drop" of
paint landing on that pixel during frame output. If any pixel is missed, it will
not be painted, and will be shown multiple times with the last drop of paint
that it received (most likely from somewhere else on the screen).<br><br>
There are 2 simple ways to be sure that every pixel is painted by at least 1
layer of paint. The first alternative is to create a text terminal primitive, or a custom
tile array, that covers the entire screen. The terminal, if full-screen, implies a tile array of characters in 8x8-pixel cells, totalling 100
columns across by 75 rows down. Every pixel on the screen is covered by such a
terminal primitive (at present there is no way to make it smaller; that is
a future enhancement). The second alternative is to create a solid rectangle primitive with the
X/Y coordinates at (0, 0), a width of 800 pixels, and a height of 600 pixels.
Such a rectangle can be any of the 64 colors, and would serve as a background
for the entire video frame. Other things could be drawn on top of it.<br><br>
Of course, you have the option of piecing together various primitives of
your choosing, as long as you cover the entire screen.<br><br>
To be clear, if the EZ80 application does not define some kind of background
(often, either a full tile array or a solid rectangle), then some part(s) of the
screen will not be painted properly, giving undesirable effects.

# OTF Dynamic Code Generation

As mentioned above, the OTF mode uses some built-in ESP32 assembler functions to help with drawing primitives quickly; however, it
goes a step further. In order to reduce the number of decisions made
while drawing, it uses the primitive definitions to generate portions
of ESP32 code at runtime. In some cases, instructions are generated
that call built-in functions in whatever sequence is required.
In other cases, instructions are generated that write to the specific
pixel data bytes.
<br><br>
For example, to draw a 20 pixel long line from X position 39 to X position 58, inclusive, the OTF mode will generate instructions to
set the single pixel at 39, set the 16 pixels from 40 to 55 (4 pixels at a time), set 2 pixels at 56 to 57, and finally, set the remaining
single pixel at 58.
<br><br>
The code generation operation itself does take a small amount of time,
and depending on how many primitives are processed to do so, there
may be some temporary effect on painting the screen, meaning
that it may flicker or duplicate scan lines during that time.


# OTF Function Codes

The OTF mode uses <b>VDU 23, 0, 30</b> as its command signature for defining and processing drawing primitives. The following list gives an overview of the available commands in OTF mode; <b>however, not all of these commands have
been implemented yet, and this section is subject to change!</b>
Each of these commands will be explained in detail once the code has all been written and tested.

## Set flags for primitive
<b>VDU 23, 30, 0, id; flags;</b>

This command modifies certain flag bits for a primitive. Some flag bits are fixed upon creation
of the primitive, and cannot be changed. The following bits can be changed.

## Set primitive position
<b>VDU 23, 30, 1, id; x; y;</b>

If a primitive is not using the "absolute position" flag, then
this command sets the relative position of the primitive, with respect to its parent's position. If the
parent is at (100, 100), then setting the primitive's position to (5, 5) will
place the primitive at position (105, 105), relative to the primitive's grandparent (if any),
or in other words, at a 5-pixel offset from its parent, in X and Y.

If a primitive is using the "absolute position" flag, then this command
sets the absolute position of the primitive, and that position is relative to the screen,
not to the parent.

## Adjust primitive position
<b>VDU 23, 30, 2, id; x; y;</b>

If a primitive is not using the "absolute position" flag, then
this command ajusts the relative position of the primitive, with respect to its parent's position. If the
parent is at (100, 100), and the primitive is at (5, 5) then adjusting the primitive's position by (2, 2) will
place the primitive at position (107, 107), relative to the primitive's grandparent (if any),
or in other words, at a 7-pixel offset from its parent, in X and Y.

If a primitive is using the "absolute position" flag, then this command
adjusts the absolute position of the primitive, and that position is relative to the screen,
not to the parent.

## Delete primitive
<b>VDU 23, 30, 3, id;</b>

This command deletes the primitive, and any children that it has. If
the primitive, such as a tile map, holds its own bitmaps, those will be
deleted with the primitive. Bear in mind that you can hide a
primitive by changing its flags, while still keeping it intact.

## Create primitive: Point
<b>VDU 23, 30, 4, id; pid; flags; x; y; c</b>

This command creates a primitive that draws a point (sets a single pixel).

## Create primitive: Line
<b>VDU 23, 30, 5, id; pid; flags; x1; y1; x2; y2; c</b>

This commmand creates a primitive that draws a line. The endpoints
are included (i.e., are drawn).

## Create primitive: Triangle Outline
<b>VDU 23, 30, 6, id; pid; flags; x1; y1; x2; y2; x3; y3; c</b>

This commmand creates a primitive that draws the outline of a triangle. The triangle is not filled.

## Create primitive: Solid Triangle
<b>VDU 23, 30, 7, id; pid; flags; x1; y1; x2; y2; x3; y3; c</b>

This commmand creates a primitive that draws a solid, filled
traingle. The triangle does not have a distinct outline with
a different color than the fill color.

## Create primitive: Rectangle Outline
<b>VDU 23, 30, 8, id; pid; flags; x; y; w; h; c</b>

This commmand creates a primitive that draws the outline of a rectangle. The rectangle is not filled. Note that the width and
height are given, not the diagonal coordinates.

## Create primitive: Solid Rectangle
<b>VDU 23, 30, 9, id; pid; flags; x; y; w; h; c</b>

This commmand creates a primitive that draws a solid, filled rectangle.
The rectangle does not have a distinct outline with a different
color than the fill color.
Note that the width and height are given, not the diagonal coordinates.

## Create primitive: Ellipse Outline
<b>VDU 23, 30, 10, id; pid; flags; x; y; w; h; c</b>

This commmand creates a primitive that draws the outline of an ellipse. The ellipse is not filled. Note that width and height
are given, not the diagonal coordinates.

## Create primitive: Solid Ellipse
<b>VDU 23, 30, 11, id; pid; flags; x; y; w; h; c</b>

This commmand creates a primitive that draws a solid, filled ellipse.
The ellipse does not have a distinct outline with a different
color than the fill color.
Note that width and height are given, not
the diagonal coordinates.

## Create primitive: Tile Map
<b>VDU 23, 30, 12, id; pid; flags; cols; rows; w; h;</b>

This commmand creates a primitive that draws a sparse tile map,
as opposed to a full or mostly full tile array.
The number of cells in the map is equal to the number of rows
multiplied by the number of columns. The given width and height
specify the size of a single tile. The overall width of the tile
map is equal to the width of one tile multiplied by the number
of columns. The overall height of the tile map is equal to the
height of one tile multiplied by the number of rows.

The given width must be a multiple of 4 pixels. The given height can be any positive number, bearing in mind the memory requirements for
storing bitmaps.

The tile map will store its own setup of bitmaps, which are distinct
from any bitmaps created as individual primitives. If the
tile map is deleted, its owned bitmaps are deleted with it.

A sparse tile map is intended to be used when the ratio of defined
(used) cells to undefined (unused) cells is rather low. Since it
is implemented as a map, processing it is slower than using a
tile array. Thus, if you want a set of tiles with a high used-to-unused ratio,
it would be more efficient to use a tile array. Slower processing
might cause flicker.

## Create primitive: Tile Array
<b>VDU 23, 30, 13, id; pid; flags; cols; rows; w; h;</b>

This commmand creates a primitive that draws a full or mostly full tile array, as opposed to a sparse tile map.
The number of cells in the array is equal to the number of rows
multiplied by the number of columns. The given width and height
specify the size of a single tile. The overall width of the tile
array is equal to the width of one tile multiplied by the number
of columns. The overall height of the tile array is equal to the
height of one tile multiplied by the number of rows.

The given width must be a multiple of 4 pixels. The given height can be any positive number, bearing in mind the memory requirements for
storing bitmaps.

The tile array will store its own setup of bitmaps, which are distinct
from any bitmaps created as individual primitives. If the
tile array is deleted, its owned bitmaps are deleted with it.

A tile array is intended to be used when the ratio of defined
(used) cells to undefined (unused) cells is rather high. Since it
is implemented as an array, processing it is faster than using a
tile map. Thus, if you want a set of tiles with a low used-to-unused ratio,
consider using a tile map, to avoid wasting space for unused cells.

## Create primitive: Solid Bitmap
<b>VDU 23, 30, 14, id; pid; flags; w; h;</b>

This commmand creates a primitive that draws a solid bitmap, meaning
that every pixel is fully opaque (though each pixel has its own color).
A solid bitmap may be the most efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower, and their overuse could cause flicker.

## Create primitive: Masked Bitmap
<b>VDU 23, 30, 15, id; pid; flags; w; h;

This commmand creates a primitive that draws a masked bitmap, meaning
that every pixel is either fully opaque (though each pixel has its own color) or fully transparent.
A solid bitmap may be the most efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower, and their overuse could cause flicker.

## Create primitive: Transparent Bitmap
<b>VDU 23, 30, 16, id; pid; flags; w; h; c</b>

This commmand creates a primitive that draws a transparent bitmap, meaning
that each pixel has either 0%, 25%, 50%, 75%, or 100% opacity.
A transparent bitmap may be the least efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower than solid bitmaps, and their overuse could cause flicker.

## Create primitive: Group
<b>VDU 23, 30, 17, id; pid; flags; x; y;</b>

This commmand creates a primitive that groups its child primitives,
for the purposes of motion and clipping. If a group node has
children, and the group node is moved, and the children are positioned
relative to their parent (i.e., do not use the "absolute position" flag),
then the children are moved with the parent. Note that a group node
has no visible representation (i.e., is not drawn).

Changing the flags of a group node can show or hide its children.

## Set position & slice solid bitmap
<b>VDU 23, 30, 18, id; x; y; s; h;</b>

This command sets the position of a solid bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Set position & slice masked bitmap
<b>VDU 23, 30, 19, id; x; y; s; h;</b>

This command sets the position of a masked bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Set position & slice transparent bitmap
<b>VDU 23, 30, 20, id; x; y; s; h;</b>

This command sets the position of a transparent bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Adjust position & slice solid bitmap
<b>VDU 23, 30, 21, id; x; y; s; h;</b>

This command adjusts the position of a solid bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Adjust position & slice masked bitmap
<b>VDU 23, 30, 22, id; x; y; s; h;</b>

This command adjusts the position of a masked bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Adjust position & slice transparent bitmap
<b>VDU 23, 30, 23, id; x; y; s; h;</b>

This command adjusts the position of a transparent bitmap, and specifies
the starting vertical offset within the bitmap, plus the height
to draw. It not necessary to draw the entire bitmap. This command
may be used to select the individual frames of a sprite, or to
scroll a bitmap vertically within a specified height.

By default, when a bitmap is created, its starting vertical offset
is zero, and its draw height is equal to its created height.

## Set solid bitmap pixel
<b>VDU 23, 30, 24, id; x; y; c</b>

This command sets the color of a single pixel within a solid bitmap.

## Set masked bitmap pixel
<b>VDU 23, 30, 25, id; x; y; c</b>

This command sets the color of a single pixel within a masked bitmap.

## Set transparent bitmap pixel
<b>VDU 23, 30, 26, id; x; y; c</b>

This command sets the color of a single pixel within a transparent bitmap.

## Set solid bitmap pixels
<b>VDU 23, 30, 27, id; x; y; n; c0, c1, c2, ...</b>

This command sets the colors of multiple pixels within a solid bitmap.

## Set masked bitmap pixels
<b>VDU 23, 30, 28, id; x; y; n; c0, c1, c2, ...</b>

This command sets the colors of multiple pixels within a masked bitmap.

## Set transparent bitmap pixels
<b>VDU 23, 30, 29, id; x; y; n; c0, c1, c2, ...</b>

This command sets the colors of multiple pixels within a transparent bitmap.

## Set image ID for tile in tile map
<b>VDU 23, 30, 30, id; col; row; img;</b>

This command specifies which bitmap should be draw in a specific
cell of a tile map. The bitmap must have been created already.

## Set image ID for tile in tile array
<b>VDU 23, 30, 31, id; col; row; img;</b>

This command specifies which bitmap should be draw in a specific
cell of a tile array. The bitmap must have been created already.

## Set image pixel in tile map
<b>VDU 23, 30, 32, id; img; x; y; c</b>

This command sets the color of a single pixel within a tile map.

## Set image pixels in tile map
<b>VDU 23, 30, 33, id; img; x; y; n; c0, c1, c2, ...</b>

This command sets the colors of multiple pixels within a tile map.

## Create primitive: Triangle List Outline
<b>VDU 23, 30, 34, id; pid; flags, n; c, x1; y1; ... xn; yn</b>

This command creates a series of triangle outlines.

A triangle list is a series of triangles that do not necessarily share points, but could, if those points are duplicated. They may be located together or apart. For each triangle, its 3 points must be specified.

## Create primitive: Solid Triangle List
<b>VDU 23, 30, 35, id; pid; flags, n; c, x1; y1; ... xn; yn;</b>

## Create primitive: Triangle Fan Outline
<b>VDU 23, 30, 36, id; pid; flags, n; c, sx0; sy0; sx1; sy1; ... xn; yn;</b>

A triangle fan is a series of triangles that share a common center point, and each 2 consecutive triangles share an edge point.

## Create primitive: Solid Triangle Fan
<b>VDU 23, 30, 37, id; pid; flags, n; c, sx0; sy0; sx1; sy1; ... xn; yn;</b>

## Create primitive: Triangle Strip Outline
<b>VDU 23, 30, 38, id; pid; flags, n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn;</b>

A triangle strip is a series of triangles where each 2 consecutive triangles share 2 common points.

## Create primitive: Solid Triangle Strip
<b>VDU 23, 30, 39, id; pid; flags, n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn;</b>

## Create primitive: Quad Outline
<b>VDU 23, 30, 40, id; pid; flags, c, x1; y1; x2; y2; x3; y3; x4; y4;</b>

A quad is  4-sided, convex polygon that does not necessarily have any internal right angles, but could.

## Create primitive: Solid Quad
<b>VDU 23, 30, 41, id; pid; flags, c, x1; y1; x2; y2; x3; y3; x4; y4;</b>

## Create primitive: Quad List Outline
<b>VDU 23, 30, 42, id; pid; flags, n; c, x1; y1; ... xn; yn;</b>

A quad list is a series of quads that do not necessarily share points, but could, if those points are duplicated. They may be located together or apart. For each quad, its 4 points must be specified.

## Create primitive: Solid Quad List
<b>VDU 23, 30, 43, id; pid; flags, n; c, x1; y1; ... xn; yn;</b>

## Create primitive: Quad Strip Outline
<b>VDU 23, 30, 44, id; pid; flags, n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn;</b>

## Create primitive: Solid Quad Strip
<b>VDU 23, 30, 45, id;800x600x64 On-the-Fly Command Set:
 pid; flags, n; c, x1; y1; ... xn; yn;</b>

