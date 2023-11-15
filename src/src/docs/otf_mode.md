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

# OTF Function Codes

The OTF mode uses <b>VDU 23, 30</b> as its command signature for defining and processing drawing primitives.
Most links below point to document sections describing the available commands in OTF mode;
<b>however, not all of these commands have
been implemented yet, and the sections are subject to change!</b>

# Document Sections

<br>[Bitmap Primitive](otf_bitmap.md)
<br>[Code Generation](otf_code_gen.md)
<br>[Ellipse Primitive](otf_ellipse.md)
<br>[Group Primitive](otf_group.md)
<br>[Line Primitive](otf_line.md)
<br>[OTF Colors](otf_colors.md)
<br>[OTF Critical Section](otf_critical.md)
<br>[OTF Strategy](otf_strategy.md)
<br>[Point Primitive](otf_point.md)
<br>[Primitive Flags](otf_flags.md)
<br>[Rectangle Primitive](otf_rectangle.md)
<br>[Terminal Primitive](otf_terminal.md)
<br>[Tile Array Primitive](otf_tile_array.md)
<br>[Tile Map Primitive](otf_tile_map.md)
<br>[Triangle Primitive](otf_triangle.md)
