// di_primitive_const.INC - Structure member offsets for base drawing primitives
// NOTE: This file must track exactly with structures in di_primitive.h.
//
// A drawing primitive tells how to draw a particular type of simple graphic object.
//
// Copyright (c) 2023 Curtis Whitley
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// 

// Offsets into DiPrimitive base class structure:

                        // offset 0 is the location of the vtable pointer
m_view_x = 4            // upper-left x coordinate of the enclosing viewport, relative to the screen
m_view_y = 8            // upper-left y coordinate of the enclosing viewport, relative to the screen
m_view_x_extent = 12    // lower-right x coordinate plus 1, of the enclosing viewport
m_view_y_extent = 16    // lower-right y coordinate plus 1, of the enclosing viewport
m_rel_x = 20            // upper-left x coordinate, relative to the parent
m_rel_y = 24            // upper-left y coordinate, relative to the parent
m_rel_dx = 28           // auto-delta-x as a 16-bit fraction, relative to the parent
m_rel_dy = 32           // auto-delta-y as a 16-bit fraction, relative to the parent
m_auto_moves = 36       // number of times to move this primitive automatically
m_abs_x = 40            // upper-left x coordinate, relative to the screen
m_abs_y = 44            // upper-left y coordinate, relative to the screen
m_width = 48            // coverage width in pixels
m_height = 52           // coverage height in pixels
m_x_extent = 56         // sum of m_abs_x + m_width
m_y_extent = 60         // sum of m_abs_y + m_height
m_color = 64            // applies to some primitives, but not to others
m_future32 = 68         // for potential future use
m_parent = 72           // id of parent primitive
m_first_child = 76      // id of first child primitive
m_last_child = 80       // id of last child primitive
m_prev_sibling = 84     // id of previous sibling primitive
m_next_sibling = 88     // id of next sibling primitive
m_first_group = 92      // lowest index of drawing group in which it is a member
m_last_group = 94       // highest index of drawing group in which it is a member
m_id = 96               // id of this primitive
m_flags = 98            // flag bits to control painting, etc.
m_future8 = 99          // for potential future use
sizeof_DiPrimitive = 100 // total size of the base class structure

// Offsets into DiPaintParams structure:

m_line32 = 0            // address of the DMA visible data
m_line8 = 4             // address of the DMA visible data
m_line_index = 8        // scan line index on the screen (0=top, 599=bottom)
