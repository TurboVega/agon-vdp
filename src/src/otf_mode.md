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
similar with bitmaps and sprites, but not with simpler primitives such as lines and triangles.

When primitives are no longer needed by the BASIC application, they may be deleted. Deleting a group
primitive will delete its children, too, reducing the number of serial commands required to delete
the parent and all of its children.

One other benefit of keeping the primitives in RAM is the ability to turn them on and off, meaning
to show them or to hide them, very easily, just by changing their flag bits. For example, a blinking
cursor could be actuated just by changing its flag bit for being painted (drawn). The cursor itself can
be any primitive (a point, line, rectangle, etc.), based on the needs of the application.

# OTF Function Codes

The OTF mode uses VDU 23, 0, 30 as its command signature for defining and processing drawing primitives. The following list gives an overview of the available commands in OTF mode.
Each of these commands will be explained in detail once the code has all been written and tested.

800x600x64 On-the-Fly Command Set:

VDU 23, 30, 0, id; flags: [6] Set flags for primitive  
VDU 23, 30, 1, id; x; y;: [9] Move primitive: absolute  
VDU 23, 30, 2, id; x; y;: [9] Move primitive: relative  
VDU 23, 30, 3, id;: [5] Delete primitive  
VDU 23, 30, 4, id; pid; flags, x; y; c: [13] Create primitive: Point  
VDU 23, 30, 5, id; pid; flags, x1; y1; x2; y2; c: [17] Create primitive: Line  
VDU 23, 30, 6, id; pid; flags, x1; y1; x2; y2; x3; y3; c: [21] Create primitive: Triangle Outline  
VDU 23, 30, 7, id; pid; flags, x1; y1; x2; y2; x3; y3; c: [21] Create primitive: Solid Triangle  
VDU 23, 30, 8, id; pid; flags, x; y; w; h; c: [17] Create primitive: Rectangle Outline  
VDU 23, 30, 9, id; pid; flags, x; y; w; h; c: [17] Create primitive: Solid Rectangle  
VDU 23, 30, 10, id; pid; flags, x; y; w; h; c: [17] Create primitive: Ellipse Outline  
VDU 23, 30, 11, id; pid; flags, x; y; w; h; c: [17] Create primitive: Solid Ellipse  
VDU 23, 30, 12, id; pid; flags, cols; rows; bitmaps, w; h;: [17] Create primitive: Tile Map  
VDU 23, 30, 13, id; pid; flags, w; h;: [12] Create primitive: Solid Bitmap  
VDU 23, 30, 14, id; pid; flags, w; h;: [12] Create primitive: Masked Bitmap  
VDU 23, 30, 15, id; pid; flags, w; h; c: [13] Create primitive: Transparent Bitmap  
VDU 23, 30, 16, id; pid; flags, x; y;: [12] Create primitive: Group  
VDU 23, 30, 17, id; x; y; s; h;: [13] Move & slice solid bitmap: absolute  
VDU 23, 30, 18, id; x; y; s; h;: [13] Move & slice masked bitmap: absolute  
VDU 23, 30, 19, id; x; y; s; h;: [13] Move & slice transparent bitmap: absolute  
VDU 23, 30, 20, id; x; y; s; h;: [13] Move & slice solid bitmap: relative  
VDU 23, 30, 21, id; x; y; s; h;: [13] Move & slice masked bitmap: relative  
VDU 23, 30, 22, id; x; y; s; h;: [13] Move & slice transparent bitmap: relative  
VDU 23, 30, 23, id; x; y; c: [10] Set solid bitmap pixel  
VDU 23, 30, 24, id; x; y; c: [10] Set masked bitmap pixel  
VDU 23, 30, 25, id; x; y; c: [10] Set transparent bitmap pixel  
VDU 23, 30, 26, id; x; y; n; c0, c1, c2, ...: [11+n] Set solid bitmap pixels  
VDU 23, 30, 27, id; x; y; n; c0, c1, c2, ...: [11+n] Set masked bitmap pixels  
VDU 23, 30, 28, id; x; y; n; c0, c1, c2, ...: [11+n] Set transparent bitmap pixels  
VDU 23, 30, 29, id; col; row; bi: [10] Set bitmap index for tile in tile map  
VDU 23, 30, 30, id; bi, x; y; c: [11] Set bitmap pixel in tile map  
VDU 23, 30, 31, id; bi, x; y; n; c0, c1, c2, ...: [12+n] Set bitmap pixels in tile map  


