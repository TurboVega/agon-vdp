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
#define FLD_view_x  4            // upper-left x coordinate of the enclosing viewport, relative to the screen
#define FLD_view_y  8            // upper-left y coordinate of the enclosing viewport, relative to the screen
#define FLD_view_x_extent  12    // lower-right x coordinate plus 1, of the enclosing viewport
#define FLD_view_y_extent  16    // lower-right y coordinate plus 1, of the enclosing viewport
#define FLD_rel_x  20            // upper-left x coordinate, relative to the parent
#define FLD_rel_y  24            // upper-left y coordinate, relative to the parent
#define FLD_rel_dx  28           // auto-delta-x as a 16-bit fraction, relative to the parent
#define FLD_rel_dy  32           // auto-delta-y as a 16-bit fraction, relative to the parent
#define FLD_auto_moves  36       // number of times to move this primitive automatically
#define FLD_abs_x  40            // upper-left x coordinate, relative to the screen
#define FLD_abs_x_word  44       // m_abs_x & 0xFFFFFFFC (word boundary)
#define FLD_abs_y  48            // upper-left y coordinate, relative to the screen
#define FLD_width  52            // coverage width in pixels
#define FLD_height  56           // coverage height in pixels
#define FLD_x_extent  60         // sum of m_abs_x + m_width
#define FLD_y_extent  64         // sum of m_abs_y + m_height
#define FLD_draw_x  68           // max of m_abs_x and m_view_x
#define FLD_draw_y  72           // max of m_abs_y and m_view_y
#define FLD_draw_x_extent  76    // min of m_x_extent and m_view_x_extent
#define FLD_draw_y_extent  80    // min of m_y_extent and m_view_y_extent
#define FLD_draw_x_offset  84    // difference of m_draw_x - m_abs_x
#define FLD_draw_y_offset  88    // difference of m_draw_y - m_abs_y
#define FLD_draw_x_word  92      // m_draw_x & 0xFFFFFFFC (word boundary)
#define FLD_draw_x_word_offset  96 // difference of m_draw_x_word - m_abs_x_word
#define FLD_color  100           // applies to some primitives, but not to others
#define FLD_future32  104        // for potential future use
#define FLD_parent  108          // id of parent primitive
#define FLD_first_child  112     // id of first child primitive
#define FLD_last_child  116      // id of last child primitive
#define FLD_prev_sibling  120    // id of previous sibling primitive
#define FLD_next_sibling  124    // id of next sibling primitive

#define FLD_first_group  128     // lowest index of drawing group in which it is a member
#define FLD_last_group  130      // highest index of drawing group in which it is a member
#define FLD_id  132              // id of this primitive
#define FLD_flags  134           // flag bits to control painting, etc.
#define sizeof_DiPrimitive  136  // total size of the base class structure
