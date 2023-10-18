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

# OTF Primitve Flags

The OTF mode uses flags to control certain aspects of primitives.
Here is an overview of the current set of flags (which is always
subject to additions in the future):

```
#define PRIM_FLAG_PAINT_THIS  0x0001  // whether to paint this primitive
#define PRIM_FLAG_PAINT_KIDS  0x0002  // whether to paint child primitives
#define PRIM_FLAG_CLIP_THIS   0x0004  // whether to clip this primitive
#define PRIM_FLAG_CLIP_KIDS   0x0008  // whether to clip child primitives
#define PRIM_FLAG_H_SCROLL    0x0010  // whether to support horizontal scrolling
#define PRIM_FLAG_ABSOLUTE    0x0040  // whether to use absolute coordinates always
#define PRIM_FLAGS_MASKED     0x0080  // hint that pixels are fully opaque or transparent
#define PRIM_FLAGS_BLENDED    0x0100  // hint that pixels may be blended
#define PRIM_FLAGS_ALL_SAME   0x0200  // hint that all lines can be drawn the same way
#define PRIM_FLAGS_CAN_DRAW   0x1000  // whether this primitive can be drawn at all
#define PRIM_FLAGS_X          0x2000  // hint that x will be given
#define PRIM_FLAGS_X_SRC      0x4000  // hint that x and src pixel ptr will be given
#define PRIM_FLAGS_DEFAULT    0x000F  // flags set when a new base primitive is constructed
#define PRIM_FLAGS_CHANGEABLE 0x000F  // flags that the app can change after primitive creation
```

The following describes the individual flags in more detail.

<b>PRIM_FLAG_PAINT_THIS</b><br>
This flag determines whether the primitive is painted (drawn).
It is essentially a <i>visibility</i> flag, because it partially
determines whether the primitive can be visible. Other factors,
such as position may also affect visibility. Inverting this flag
on a periodic basis will cause the primitive to blink. 

<b>PRIM_FLAG_PAINT_KIDS</b><br>
This flag determines whether this children of the primitive,
if any, are painted (drawn). This flag controls whether drawing
traverses lower levels of the primitive tree (nesting), below
the primitive itself. Note that even if this flag is set,
a child primitive will not be drawn if its own PRIM_FLAG_PAINT_THIS
flag is cleared. Inverting this flag on a periodic basis will
cause child primitives to blink.

<b>PRIM_FLAG_CLIP_THIS</b><br>
This flag determines whether this primitive is clipped by the draw
region of its parent. If this flag is cleared, the primitive
is clipped by the screen.
Presently, the draw region of any primitive
is defined when it is created, based on the designated size at
that time. There is currently no way to resize a primitive after
it has been created. It is possible to use a group primitive to
define a clipping region for child primitives. The group itself
has no visible aspect, but can affect its children. For example,
a group of spaceships can be moved together by moving their
parent group, if created as children of that group. It is possible
to simulate growing or shrinking by using multiple primitives,
and only displaying one at a time.

<b>PRIM_FLAG_CLIP_KIDS</b><br>
This flag determines whether to clip child primitives by the parent
primitive's draw region. If this flag is clear, the children are
clipped by their grandparent, if any, or by the screen, not by their parent.

<b>PRIM_FLAG_H_SCROLL</b><br>
This flag determines whether the primitive will need to be scrolled
horizontally on a 1-pixel alignment. This flag does not need to be
specified in order to scroll on a 4-pixel alignment. If this flag
is set when creating the primitive, more RAM will be used to support the smooth horizontal motion.

<b>PRIM_FLAG_ABSOLUTE</b><br>
This flag determines whether the primitive should <i>always</i> be
positioned relative to the screen, and not to any of its ancestors.
If this flag is cleared, the displayed location of the primitive
may be affected by the locations of its parent, grandparent, etc.

<b>PRIM_FLAGS_MASKED</b><br>
This flag determines whether a primitive is intended to be drawn
in a masked manner, as opposed to a solid manner. In a solid, all
pixels are 100% opaque. In a mask, all pixels are either 100%
opaque or 0% opaque (100% transparent). This flag is used
for bitmaps, and helps the OTF mode to optimize drawing speed.

<b>PRIM_FLAGS_BLENDED</b><br>
This flag determines whether a primitive is intended to be drawn
in a blended manner, as opposed to a solid manner. In a solid, all
pixels are 100% opaque. In a blend, all pixels are either 100%,
75%, 50%, 25%, or 0% opaque (0%, 25%, 50%, 75%, or 100% transparent,
respectively). This flag is used
for bitmaps, and helps the OTF mode to optimize drawing speed. It
may also be used for other primitives such as lines and triangles.

<b>PRIM_FLAGS_ALL_SAME</b><br>
This flag determines whether all of the scan lines in the primitive
can be drawn using the same dynamic instructions. It is used for
rectangles and bitmaps, and helps the OTF mode to optimize drawing
speed, plus reduces the RAM used. It cannot be used with blending,
unless <i>all</i> pixels in the primitive are at the same opacity
level (e.g., all pixels are 50% opaque).

<b>PRIM_FLAGS_CAN_DRAW</b><br>
This OTF-internal flag determines whether a primitive can be
drawn based on various factors, such as position, clipping, and
transparency. Its value is determined dynamically by OTF.

<b>PRIM_FLAGS_X</b><br>
This OTF-internal flag determines whether the X coordinate is provided,
as opposed to being fixed, when each scan line of a primitive
is drawn. This flag and some related code exist for potential future use.

<b>PRIM_FLAGS_X_SRC</b><br>
This OTF-internal flag determines whether the X coordinate is provided,
as opposed to being fixed, when each scan line of a primitive
is drawn. An example of where this is required is when drawing
a tile array or a tile map. OTF will set this flag automatically
in such cases, because it affects dynamic code generation.

<b>PRIM_FLAGS_DEFAULT</b><br>
This flag illustrates which flags are typically set when
creating a primitive. Most often, when creating a primitive,
such as a triangle, you would pass a value of 15 (0x000F)
as the flags parameter. You may want to use another value,
if you intend to create, but not immediately display,
the new primitive.

<b>PRIM_FLAGS_CHANGEABLE</b><br>
This flag illustrates which flags my be changed <i>after</i> the
primitive is created. Setting or clearing any of these flags
may have an affect on how or whether the primitive is drawn.

# OTF Colors

The OTF mode can display all 64 colors that the Agon is capable
of showing. Since the OTF mode supports transparency, it uses
the upper 2 bits of the color byte for that. In other words,
it uses an AABBGGRR format, where the 8 bits of the color byte
are concerned. The alpha (opacity) values are as follows:

00BBGGRR - the pixel is 25% opaque (75% transparent)
01BBGGRR - the pixel is 50% opaque (50% transparent)
10BBGGRR - the pixel is 75% opaque (25% transparent)
11BBGGRR - the pixel is 100% opaque (0% transparent)

The obvious question is, how do we specify a pixel that is 0%
opaque (100% transparent)? That is done by choosing a byte value
that will not be used in the primitive to represent a color. If
the primitive has a consistent opaqueness (all visible pixels are at
the same opacity level), then the chosen byte value can be any value
that does not specify that particular opacity level. For example,
if all pixels are supposed to be 75% opaque (alpha 10 in binary),
then the value 0 (0x00) works just fine as the transparent
color, because it does not indicate 75% opacity.

The potential conflict arises when the primitive, such as a
bitmap, is intended to have variable blending, meaning that each
pixel can have its own opacity level. In that case, one of the
255 possible byte values must be sacrificed, and used as the
fully transparent color. Simply put, you must choose a value
that does not exist in the source bitmap. Bear in mind that the
<i>whole</i> byte value is used to represent an invisible pixel,
not just the color bits. This means that the transparent color
can have color bits equal to an actual color in the primitive,
but it must have different alpha bits, in that case. For example,
if 0x7F (bright white at 50% opacity) is a color in the source
bitmap, you cannot use 0x7F as the transparent color, but you
could use 0xFF, if there will be no bright white pixels at
100% opacity.

# OTF Function Codes

The OTF mode uses <b>VDU 23, 0, 30</b> as its command signature for defining and processing drawing primitives. The following list gives an overview of the available commands in OTF mode; <b>however, not all of these commands have
been implemented yet, and this section is subject to change!</b>
Each of these commands will be explained in detail once the code has all been written and tested.

Here is an overview of the commands:
<br>VDU 23, 30, 0, id; flags; :  Set flags for primitive
<br>VDU 23, 30, 1, id; x; y; :  Set primitive position
<br>VDU 23, 30, 2, id; x; y; :  Adjust primitive position
<br>VDU 23, 30, 3, id; :  Delete primitive
<br>VDU 23, 30, 4, id; pid; flags; x; y; color :  Create primitive: Point
<br>VDU 23, 30, 5, id; pid; flags; x1; y1; x2; y2; color :  Create primitive: Line
<br>VDU 23, 30, 6, id; pid; flags; x1; y1; x2; y2; x3; y3; color :  Create primitive: Triangle Outline
<br>VDU 23, 30, 7, id; pid; flags; x1; y1; x2; y2; x3; y3; color :  Create primitive: Solid Triangle
<br>VDU 23, 30, 8, id; pid; flags; x; y; w; h; color :  Create primitive: Rectangle Outline
<br>VDU 23, 30, 9, id; pid; flags; x; y; w; h; color :  Create primitive: Solid Rectangle
<br>VDU 23, 30, 10, id; pid; flags; x; y; w; h; color :  Create primitive: Ellipse Outline
<br>VDU 23, 30, 11, id; pid; flags; x; y; w; h; color :  Create primitive: Solid Ellipse
<br>VDU 23, 30, 12, id; pid; flags; cols; rows; w; h; :  Create primitive: Tile Map
<br>VDU 23, 30, 13, id; pid; flags; cols; rows; w; h; :  Create primitive: Tile Array
<br>VDU 23, 30, 14, id; pid; flags; w; h; :  Create primitive: Solid Bitmap
<br>VDU 23, 30, 15, id; pid; flags; w; h; color :  Create primitive: Masked Bitmap
<br>VDU 23, 30, 16, id; pid; flags; w; h; color :  Create primitive: Transparent Bitmap
<br>VDU 23, 30, 17, id; pid; flags; x; y; :  Create primitive: Group
<br>VDU 23, 30, 18, id; x; y; s; h; :  Set position & slice solid bitmap
<br>VDU 23, 30, 19, id; x; y; s; h; :  Set position & slice masked bitmap
<br>VDU 23, 30, 20, id; x; y; s; h; :  Set position & slice transparent bitmap
<br>VDU 23, 30, 21, id; x; y; s; h; :  Adjust position & slice solid bitmap
<br>VDU 23, 30, 22, id; x; y; s; h; :  Adjust position & slice masked bitmap
<br>VDU 23, 30, 23, id; x; y; s; h; :  Adjust position & slice transparent bitmap
<br>VDU 23, 30, 24, id; x; y; color :  Set solid bitmap pixel
<br>VDU 23, 30, 25, id; x; y; color :  Set masked bitmap pixel
<br>VDU 23, 30, 26, id; x; y; color :  Set transparent bitmap pixel
<br>VDU 23, 30, 27, id; x; y; n; c0, c1, c2, ... :  Set solid bitmap pixels
<br>VDU 23, 30, 28, id; x; y; n; c0, c1, c2, ... :  Set masked bitmap pixels
<br>VDU 23, 30, 29, id; x; y; n; c0, c1, c2, ... :  Set transparent bitmap pixels
<br>VDU 23, 30, 30, id; pid; flags; w; h; :  Create Solid Bitmap for Tile Map
<br>VDU 23, 30, 31, id; pid; flags; w; h; color :  Create Masked Bitmap for Tile Map
<br>VDU 23, 30, 32, id; pid; flags; w; h; color :  Create Transparent Bitmap for Tile Map
<br>VDU 23, 30, 33, id; pid; flags; w; h; :  Create Solid Bitmap for Tile Array
<br>VDU 23, 30, 34, id; pid; flags; w; h; color :  Create Masked Bitmap for Tile Array
<br>VDU 23, 30, 35, id; pid; flags; w; h; color :  Create Transparent Bitmap for Tile Array
<br>VDU 23, 30, 36, id; col; row; bmid; :  Set bitmap ID for tile in tile map
<br>VDU 23, 30, 37, id; col; row; bmid; :  Set bitmap ID for tile in tile array
<br>VDU 23, 30, 38, id; bmid; x; y; color :  Set bitmap pixel in tile map
<br>VDU 23, 30, 39, id; bmid; x; y; n; c0, c1, c2, ... :  Set bitmap pixels in tile map
<br>VDU 23, 30, 40, id; pid; flags; x; y; columns; rows; :  Create primitive: Terminal
<br>VDU 23, 30, 41, id; :  Select Active Terminal
<br>VDU 23, 30, 42, id; char, fgcolor, bgcolor :  Define Terminal Character
<br>VDU 23, 30, 43, id; firstchar, lastchar, fgcolor, bgcolor :  Define Terminal Character Range
<br>VDU 23, 30, 44, id; pid; flags; n; c, x1; y1; ... xn; yn; :  Create primitive: Triangle List Outline
<br>VDU 23, 30, 45, id; pid; flags; n; c, x1; y1; ... xn; yn; :  Create primitive: Solid Triangle List
<br>VDU 23, 30, 46, id; pid; flags; n; c, sx0; sy0; sx1; sy1; ... xn; yn; :  Create primitive: Triangle Fan Outline
<br>VDU 23, 30, 47, id; pid; flags; n; c, sx0; sy0; sx1; sy1; ... xn; yn; :  Create primitive: Solid Triangle Fan
<br>VDU 23, 30, 48, id; pid; flags; n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn; :  Create primitive: Triangle Strip Outline
<br>VDU 23, 30, 49, id; pid; flags; n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn; :  Create primitive: Solid Triangle Strip
<br>VDU 23, 30, 50, id; pid; flags; c, x1; y1; x2; y2; x3; y3; x4; y4; :  Create primitive: Quad Outline
<br>VDU 23, 30, 51, id; pid; flags; c, x1; y1; x2; y2; x3; y3; x4; y4; :  Create primitive: Solid Quad
<br>VDU 23, 30, 52, id; pid; flags; n; c, x1; y1; ... xn; yn; :  Create primitive: Quad List Outline
<br>VDU 23, 30, 53, id; pid; flags; n; c, x1; y1; ... xn; yn; :  Create primitive: Solid Quad List
<br>VDU 23, 30, 54, id; pid; flags; n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn; :  Create primitive: Quad Strip Outline
<br>VDU 23, 30, 55, id; pid; flags; n; c, x1; y1; ... xn; yn; :  Create primitive: Solid Quad Strip

## Set flags for primitive
<b>VDU 23, 30, 0, id; flags;</b>

This command modifies certain flag bits for a primitive. Some flag bits are fixed upon creation
of the primitive, and cannot be changed. Refer to the OTF Primitive
Flags section, above, for more information.

## Set primitive position
<b>VDU 23, 30, 1, id; x; y;</b>

If a primitive is not using the PRIM_FLAG_ABSOLUTE flag, then
this command sets the relative position of the primitive, with respect to its parent's position. If the
parent is at (100, 100), then setting the primitive's position to (5, 5) will
place the primitive at position (105, 105), relative to the primitive's grandparent (if any),
or in other words, at a 5-pixel offset from its parent, in X and Y.

If a primitive is using the PRIM_FLAG_ABSOLUTE flag, then this command
sets the absolute position of the primitive, and that position is relative to the screen,
not to the parent.

## Adjust primitive position
<b>VDU 23, 30, 2, id; x; y;</b>

If a primitive is not using the PRIM_FLAG_ABSOLUTE flag, then
this command ajusts the relative position of the primitive, with respect to its parent's position. If the
parent is at (100, 100), and the primitive is at (5, 5) then adjusting the primitive's position by (2, 2) will
place the primitive at position (107, 107), relative to the primitive's grandparent (if any),
or in other words, at a 7-pixel offset from its parent, in X and Y.

If a primitive is using the PRIM_FLAG_ABSOLUTE flag, then this command
adjusts the absolute position of the primitive, and that position is relative to the screen,
not to the parent.

## Delete primitive
<b>VDU 23, 30, 3, id;</b>

This command deletes the primitive, and any children that it has. If
the primitive, such as a tile map, holds its own bitmaps, those will be
deleted with the primitive. Bear in mind that you can hide a
primitive by changing its flags, while still keeping it intact.

## Create primitive: Point
<b>VDU 23, 30, 4, id; pid; flags; x; y; color</b>

This command creates a primitive that draws a point (sets a single pixel).

## Create primitive: Line
<b>VDU 23, 30, 5, id; pid; flags; x1; y1; x2; y2; color</b>

This commmand creates a primitive that draws a line. The endpoints
are included (i.e., are drawn).

## Create primitive: Triangle Outline
<b>VDU 23, 30, 6, id; pid; flags; x1; y1; x2; y2; x3; y3; color</b>

This commmand creates a primitive that draws the outline of a triangle. The triangle is not filled.

## Create primitive: Solid Triangle
<b>VDU 23, 30, 7, id; pid; flags; x1; y1; x2; y2; x3; y3; color</b>

This commmand creates a primitive that draws a solid, filled
traingle. The triangle does not have a distinct outline with
a different color than the fill color.

## Create primitive: Rectangle Outline
<b>VDU 23, 30, 8, id; pid; flags; x; y; w; h; color</b>

This commmand creates a primitive that draws the outline of a rectangle. The rectangle is not filled. Note that the width and
height are given, not the diagonal coordinates.

## Create primitive: Solid Rectangle
<b>VDU 23, 30, 9, id; pid; flags; x; y; w; h; color</b>

This commmand creates a primitive that draws a solid, filled rectangle.
The rectangle does not have a distinct outline with a different
color than the fill color.
Note that the width and height are given, not the diagonal coordinates.

## Create primitive: Ellipse Outline
<b>VDU 23, 30, 10, id; pid; flags; x; y; w; h; color</b>

This commmand creates a primitive that draws the outline of an ellipse. The ellipse is not filled. Note that width and height
are given, not the diagonal coordinates.

## Create primitive: Solid Ellipse
<b>VDU 23, 30, 11, id; pid; flags; x; y; w; h; color</b>

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

Refer to the section on OTF Primitive Flags for information about
certain flags that are useful for tile maps.

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
tile map. The trade-off is that the entire array is allocated
from RAM, so it may be more expensive than a sparse tile map.
Thus, if you want a set of tiles with a low used-to-unused ratio,
consider using a tile map, to avoid wasting space for unused cells.

Even the a tile array is fully allocated, each cell is just a pointer
to the pixel data for some tile bitmap, and that pointer can be NULL,
in which case, nothing is drawn for the corresponding cell. In that
manner, you can create an array that appears to be sparse. Such is
the case with random-length text lines in the terminal primitive,
which is a tile array.

## Create primitive: Solid Bitmap
<b>VDU 23, 30, 14, id; pid; flags; w; h;</b>

This commmand creates a primitive that draws a solid bitmap, meaning
that every pixel is fully opaque (though each pixel has its own color).
A solid bitmap may be the most efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower, and their overuse could cause flicker.

OTF mode will automatically set the PRIM_FLAGS_ALL_SAME flag
when this command is used.

## Create primitive: Masked Bitmap
<b>VDU 23, 30, 15, id; pid; flags; w; h; color</b>

This commmand creates a primitive that draws a masked bitmap, meaning
that every pixel is either fully opaque (though each pixel has its own color) or fully transparent.
A solid bitmap may be the most efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower, and their overuse could cause flicker.

The given color is used to represent fully transparent pixels,
so be sure to specify a byte value that is unique from any
visible color in the source bitmap. When setting the color of
each pixel in the bitmap, use that given color for any pixels
that must be invisible.

## Create primitive: Transparent Bitmap
<b>VDU 23, 30, 16, id; pid; flags; w; h; color</b>

This commmand creates a primitive that draws a transparent bitmap, meaning
that each pixel has either 0%, 25%, 50%, 75%, or 100% opacity.
A transparent bitmap may be the least efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower than solid bitmaps, and their overuse could cause flicker.

The given color is used to represent fully transparent pixels,
so be sure to specify a byte value that is unique from any
visible color in the source bitmap. When setting the color of
each pixel in the bitmap, use that given color for any pixels
that must be invisible.

OTF mode will automatically set the PRIM_FLAGS_BLENDED flag
when this command is used.

## Create primitive: Group
<b>VDU 23, 30, 17, id; pid; flags; x; y;</b>

This commmand creates a primitive that groups its child primitives,
for the purposes of motion and clipping. If a group node has
children, and the group node is moved, and the children are positioned
relative to their parent (i.e., do not use the PRIM_FLAG_ABSOLUTE flag),
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
<b>VDU 23, 30, 24, id; x; y; color</b>

This command sets the color of a single pixel within a solid bitmap.
You must use the same alpha bits for every pixel in the bitmap;
otherwise, undesirable results may occur.

## Set masked bitmap pixel
<b>VDU 23, 30, 25, id; x; y; color</b>

This command sets the color of a single pixel within a masked bitmap.
To specify a fully transparent pixel, use the same color that
was used to create the bitmap. All other pixels must have equal
alpha bits (i.e., be at the same opacity level).

## Set transparent bitmap pixel
<b>VDU 23, 30, 26, id; x; y; color</b>

This command sets the color of a single pixel within a transparent bitmap.
To specify a fully transparent pixel, use the same color that
was used to create the bitmap.

## Set solid bitmap pixels
<b>VDU 23, 30, 27, id; x; y; n; c0, c1, c2, ...</b>

This command sets the colors of multiple pixels within a solid bitmap.
As colors are processed, if the end of a scan line in the
bitmap is reached, processing moves to the first pixel in
the next scan line. Thus, it is possible to provide colors
for every pixel in the bitmap, using a single command.

## Set masked bitmap pixels
<b>VDU 23, 30, 28, id; x; y; n; c0, c1, c2, ...</b>

This command sets the colors of multiple pixels within a masked bitmap.
As colors are processed, if the end of a scan line in the
bitmap is reached, processing moves to the first pixel in
the next scan line. Thus, it is possible to provide colors
for every pixel in the bitmap, using a single command.
To specify a fully transparent pixel, use the same color that
was used to create the bitmap.

## Set transparent bitmap pixels
<b>VDU 23, 30, 29, id; x; y; n; c0, c1, c2, ...</b>

This command sets the colors of multiple pixels within a transparent bitmap.
As colors are processed, if the end of a scan line in the
bitmap is reached, processing moves to the first pixel in
the next scan line. Thus, it is possible to provide colors
for every pixel in the bitmap, using a single command.
To specify a fully transparent pixel, use the same color that
was used to create the bitmap.

## Create Solid Bitmap for Tile Map
<b>VDU 23, 30, 30, id; pid; flags; w; h;</b>

This commmand creates a solid bitmap to be used within a tile map.
Every pixel is fully opaque (though each pixel has its own color).
A solid bitmap may be the most efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower, and their overuse could cause flicker.

OTF mode will automatically set the PRIM_FLAGS_ALL_SAME flag
when this command is used.

## Create Masked Bitmap for Tile Map
<b>VDU 23, 30, 31, id; pid; flags; w; h; color</b>

This commmand creates a masked bitmap to be used within a tile map.
Every pixel is either fully opaque (though each pixel has its own color) or fully transparent.
A solid bitmap may be the most efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower, and their overuse could cause flicker.

The given color is used to represent fully transparent pixels,
so be sure to specify a byte value that is unique from any
visible color in the source bitmap. When setting the color of
each pixel in the bitmap, use that given color for any pixels
that must be invisible.

## Create Transparent Bitmap for Tile Map
<b>VDU 23, 30, 32, id; pid; flags; w; h; color</b>

This commmand creates a transparent bitmap to be used within a tile map.
Each pixel has either 0%, 25%, 50%, 75%, or 100% opacity.
A transparent bitmap may be the least efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower than solid bitmaps, and their overuse could cause flicker.

The given color is used to represent fully transparent pixels,
so be sure to specify a byte value that is unique from any
visible color in the source bitmap. When setting the color of
each pixel in the bitmap, use that given color for any pixels
that must be invisible.

OTF mode will automatically set the PRIM_FLAGS_BLENDED flag
when this command is used.

## Create Solid Bitmap for Tile Array
<b>VDU 23, 30, 33, id; pid; flags; w; h;</b>

This commmand creates a solid bitmap to be used within a tile array.
Every pixel is fully opaque (though each pixel has its own color).
A solid bitmap may be the most efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower, and their overuse could cause flicker.

OTF mode will automatically set the PRIM_FLAGS_ALL_SAME flag
when this command is used.

If you create a terminal primitive, which is a tile array, and
use the Define Terminal Character or Define Terminal Character
Range command, then the required solid bitmap(s) is(are)
created automatically.

## Create Masked Bitmap for Tile Array
<b>VDU 23, 30, 34, id; pid; flags; w; h; color</b>

<i>This command code is reserved for potential future use.
The command is not presently implemented.</i>

This commmand creates a masked bitmap to be used within a tile array.
Every pixel is either fully opaque (though each pixel has its own color) or fully transparent.
A solid bitmap may be the most efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower, and their overuse could cause flicker.

The given color is used to represent fully transparent pixels,
so be sure to specify a byte value that is unique from any
visible color in the source bitmap. When setting the color of
each pixel in the bitmap, use that given color for any pixels
that must be invisible.

## Create Transparent Bitmap for Tile Array
<b>VDU 23, 30, 35, id; pid; flags; w; h; color</b>

<i>This command code is reserved for potential future use.
The command is not presently implemented.</i>

This commmand creates a transparent bitmap to be used within a tile array.
Each pixel has either 0%, 25%, 50%, 75%, or 100% opacity.
A transparent bitmap may be the least efficient kind of bitmap, from a
processing speed perspective. Bitmaps with any transparency may be slower than solid bitmaps, and their overuse could cause flicker.

The given color is used to represent fully transparent pixels,
so be sure to specify a byte value that is unique from any
visible color in the source bitmap. When setting the color of
each pixel in the bitmap, use that given color for any pixels
that must be invisible.

OTF mode will automatically set the PRIM_FLAGS_BLENDED flag
when this command is used.

## Set bitmap ID for tile in tile map
<b>VDU 23, 30, 36, id; col; row; bmid;</b>

This command specifies which bitmap should be draw in a specific
cell of a tile map. The bitmap must have been created already.
Passing a zero for the bitmap ID will prevent the cell from
being drawn, so it will appear as an empty cell.

## Set bitmap ID for tile in tile array
<b>VDU 23, 30, 37, id; col; row; bmid;</b>

This command specifies which bitmap should be draw in a specific
cell of a tile array. The bitmap must have been created already.
Passing a zero for the bitmap ID will prevent the cell from
being drawn, so it will appear as an empty cell.

## Set bitmap pixel in tile map
<b>VDU 23, 30, 38, id; bmid; x; y; color</b>

This command sets the color of a single pixel within a
bitmap that belongs to a tile map.
To specify a fully transparent pixel, use the same color that
was used to create the tile bitmap.

## Set bitmap pixels in tile map
<b>VDU 23, 30, 39, id; bmid; x; y; n; c0, c1, c2, ...</b>

This command sets the colors of multiple pixels within a tile map.
As colors are processed, if the end of a scan line in the
bitmap is reached, processing moves to the first pixel in
the next scan line. Thus, it is possible to provide colors
for every pixel in the bitmap, using a single command.
To specify a fully transparent pixel, use the same color that
was used to create the bitmap.

## Create primitive: Terminal
<b>VDU 23, 30, 40, id; pid; flags; x; y; columns; rows;</b>

This command creates a terminal primitive, which is a tile array
used to show text characters. The terminal supports a flashing
cursor and cursor movement as characters are written, or as
directed by certain VDU commands.

You may create more than one terminal. If you have no other full
background defined, you can use a full-screen terminal as the
background, by creating it with 100 columns and 75 rows,
resulting in 800x600 pixel coverage.

At present, the only font available for a terminal is the
built-in Agon system font (8x8 pixel characters), so there
is no font specified in this command. If other fonts are used
in the future, a new command will be added to the OTF mode.

## Select Active Terminal
<b>VDU 23, 30, 41, id;</b>

This command tells the OTF manager which terminal should be the
active terminal, when more than one terminal was created. This
allows, for example, two separate areas of the screen to have
independent text (and possibly in the future, independent fonts).

## Define Terminal Character
<b>VDU 23, 30, 42, id; char, fgcolor, bgcolor</b>

This command defines a single character to be used within a terminal
primitive, meaning that it both creates a bitmap and sets the
initial pixel data of the bitmap, using the font that is
referenced by the terminal.

It is possible to use multiple different colors for the same
character glyph, in a single terminal, simply by defining
the character multiple times, using different colors.

## Define Terminal Character Range
<b>VDU 23, 30, 43, id; firstchar, lastchar, fgcolor, bgcolor</b>

This command defines a range of characters to be used within a terminal
primitive, meaning that it both creates the bitmaps and sets the
initial pixel data of the bitmaps, using the font that is
referenced by the terminal.

It is possible to use multiple different colors for the same
character glyphs, in a single terminal, simply by defining
the characters multiple times, using different colors.

## Create primitive: Triangle List Outline
<b>VDU 23, 30, 44, id; pid; flags; n; c, x1; y1; ... xn; yn</b>

This command creates a series of separate triangle outlines.

A triangle list is a series of triangles that do not necessarily share points, but could, if those points are duplicated. They may be located together or apart. For each triangle, its 3 points must be specified. The triangles are not filled.

## Create primitive: Solid Triangle List
<b>VDU 23, 30, 45, id; pid; flags; n; c, x1; y1; ... xn; yn;</b>

A triangle list is a series of triangles that do not necessarily share points, but could, if those points are duplicated. They may be located together or apart. For each triangle, its 3 points must be specified. The triangles are filled, but do not have a distinct
edge color that differs from the given color.

## Create primitive: Triangle Fan Outline
<b>VDU 23, 30, 46, id; pid; flags; n; c, sx0; sy0; sx1; sy1; ... xn; yn;</b>

A triangle fan is a series of triangles that share a common center point, and each 2 consecutive triangles share an edge point.
The triangles are not filled.

## Create primitive: Solid Triangle Fan
<b>VDU 23, 30, 47, id; pid; flags; n; c, sx0; sy0; sx1; sy1; ... xn; yn;</b>

A triangle fan is a series of triangles that share a common center point, and each 2 consecutive triangles share an edge point.
The triangles are filled, but do not have a distinct
edge color that differs from the given color.

## Create primitive: Triangle Strip Outline
<b>VDU 23, 30, 48, id; pid; flags; n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn;</b>

A triangle strip is a series of triangles where each 2 consecutive triangles share 2 common points. The triangles are not filled.

## Create primitive: Solid Triangle Strip
<b>VDU 23, 30, 49, id; pid; flags; n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn;</b>

A triangle strip is a series of triangles where each 2 consecutive triangles share 2 common points. The triangles filled, but do not have a distinct
edge color that differs from the given color.

## Create primitive: Quad Outline
<b>VDU 23, 30, 50, id; pid; flags; c, x1; y1; x2; y2; x3; y3; x4; y4;</b>

A quad is a 4-sided, convex polygon that does not necessarily have any internal right angles, but could.
The quad is not filled.

## Create primitive: Solid Quad
<b>VDU 23, 30, 51, id; pid; flags; c, x1; y1; x2; y2; x3; y3; x4; y4;</b>

A quad is a 4-sided, convex polygon that does not necessarily have any internal right angles, but could.
The quad is filled, but does not have a distinct
edge color that differs from the given color.

## Create primitive: Quad List Outline
<b>VDU 23, 30, 52, id; pid; flags; n; c, x1; y1; ... xn; yn;</b>

A quad list is a series of quads that do not necessarily share points, but could, if those points are duplicated. They may be located together or apart. For each quad, its 4 points must be specified.

## Create primitive: Solid Quad List
<b>VDU 23, 30, 53, id; pid; flags; n; c, x1; y1; ... xn; yn;</b>

A quad list is a series of quads that do not necessarily share points, but could, if those points are duplicated.
The quads are filled, but do not have a distinct
edge color that differs from the given color.

## Create primitive: Quad Strip Outline
<b>VDU 23, 30, 54, id; pid; flags; n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn;</b>

A quad strip is a series of quads where each 2 consecutive quads share 2 common points. The quads are not filled.

## Create primitive: Solid Quad Strip
<b>VDU 23, 30, 55, id; pid; flags; n; c, x1; y1; ... xn; yn;</b>

A quad strip is a series of quads where each 2 consecutive quads share 2 common points. The quads are filled, but do not have a distinct
edge color that differs from the given color.

