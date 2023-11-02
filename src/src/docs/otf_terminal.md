## Create primitive: Terminal
<b>VDU 23, 30, 150, id; pid; flags; x; y; columns; rows;</b> : Create primitive: Terminal

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

When a terminal is created, the OTF manager also creates a cursor
for it, which is a child primitive to the terminal, with an ID
equal to the terminal's ID plus one. Be sure not to use the ID
of the cursor for some other new primitive, or the cursor will
not function properly, and your new primitive may get moved
as terminal characters are displayed.

## Select Active Terminal
<b>VDU 23, 30, 151, id;</b> : Select Active Terminal

This command tells the OTF manager which terminal should be the
active terminal, when more than one terminal was created. This
allows, for example, two separate areas of the screen to have
independent text (and possibly in the future, independent fonts).

## Define Terminal Character
<b>VDU 23, 30, 152, id; char, fgcolor, bgcolor</b> : Define Terminal Character

This command defines a single character to be used within a terminal
primitive, meaning that it both creates a bitmap and sets the
initial pixel data of the bitmap, using the font that is
referenced by the terminal.

It is possible to use multiple different colors for the same
character glyph, in a single terminal, simply by defining
the character multiple times, using different colors.

## Define Terminal Character Range
<b>VDU 23, 30, 153, id; firstchar, lastchar, fgcolor, bgcolor</b> : Define Terminal Character Range

This command defines a range of characters to be used within a terminal
primitive, meaning that it both creates the bitmaps and sets the
initial pixel data of the bitmaps, using the font that is
referenced by the terminal.

It is possible to use multiple different colors for the same
character glyphs, in a single terminal, simply by defining
the characters multiple times, using different colors.

[Home](otf_mode.md)
