// di_primitive.h - Function declarations for base drawing primitives
// NOTE: This file must track exactly with constants in di_primitive_const.h.
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

#pragma once
#include <stdint.h>
#include <stddef.h>
#include "driver/gpio.h"
#include "di_constants.h"
#include "di_code.h"

#pragma pack(push,1)

class DiPrimitive {
  public:
  // An object to be drawn on the screen.
  DiPrimitive();

  // Destroys an allocated RAM required by the primitive.
  virtual ~DiPrimitive();

  // Initialize as a root primitive.
  void init_root();

  // Set the ID of this primitive as defined by the BASIC application. This
  // ID is actually the index of the primitive in a table of pointers.
  void set_id(uint16_t id);

  // Draws the primitive to the DMA scan line buffer.
  virtual void IRAM_ATTR paint(volatile uint32_t* p_scan_line, uint32_t line_index);

  // Groups scan lines for optimizing paint calls.
  bool IRAM_ATTR get_vertical_group_range(int32_t& min_group, int32_t& max_group);

  // Attach a child primitive.
  void IRAM_ATTR attach_child(DiPrimitive* child);

  // Detach a child primitive.
  void IRAM_ATTR detach_child(DiPrimitive* child);

  // Set the X, Y position relative to the parent (which may be the screen).
  virtual void IRAM_ATTR set_relative_position(int32_t rel_x, int32_t rel_y);

  // Set the delta X, Y position, relative to the parent, and the move count.
  // These values are used to update the relative position automatically, frame-by-frame.
  void IRAM_ATTR set_relative_deltas(int32_t rel_dx, int32_t rel_dy, uint32_t auto_moves);

  // Set the size of the primitive. This only used for certain types of primitives.
  virtual void IRAM_ATTR set_size(uint32_t width, uint32_t height);

  // Compute the absolute position and related data members, based on the
  // current position, relative to the parent primitive. The viewport of
  // this primitive is based on the given viewport parameters and certain flags.
  void IRAM_ATTR compute_absolute_geometry(int32_t view_x, int32_t view_y, int32_t view_x_extent, int32_t view_y_extent);

  // Clear the custom instructions needed to draw the primitive.
  virtual void IRAM_ATTR delete_instructions();
   
  // Reassemble the custom instructions needed to draw the primitive.
  virtual void IRAM_ATTR generate_instructions();

  // Convert normal alpha bits of color to opaqueness percentage.
  // This will also remove the alpha bits from the color.
  static uint8_t normal_alpha_to_opaqueness(uint8_t &color);

  // Convert inverted alpha bits of color to opaqueness percentage.  
  // This will also remove the alpha bits from the color.
  static uint8_t inverted_alpha_to_opaqueness(uint8_t &color);

  // Gets various data members.
  inline uint16_t get_id() { return m_id; }
  inline uint16_t get_flags() { return m_flags; }
  inline int32_t get_relative_x() { return m_rel_x; }
  inline int32_t get_relative_y() { return m_rel_y; }
  inline int32_t get_absolute_x() { return m_abs_x; }
  inline int32_t get_absolute_y() { return m_abs_y; }
  inline int32_t get_width() { return m_width; }
  inline int32_t get_height() { return m_height; }
  inline int32_t get_view_x() { return m_view_x; }
  inline int32_t get_view_y() { return m_view_y; }
  inline int32_t get_view_x_extent() { return m_view_x_extent; }
  inline int32_t get_view_y_extent() { return m_view_y_extent; }
  inline int32_t get_draw_x() { return m_draw_x; }
  inline int32_t get_draw_y() { return m_draw_y; }
  inline int32_t get_draw_x_extent() { return m_draw_x_extent; }
  inline int32_t get_draw_y_extent() { return m_draw_y_extent; }
  inline DiPrimitive* get_parent() { return m_parent; }
  inline DiPrimitive* get_first_child() { return m_first_child; }
  inline DiPrimitive* get_next_sibling() { return m_next_sibling; }
  inline uint8_t get_color() { return (uint8_t)m_color; }
  inline uint32_t get_color32() { return m_color; }

  // Sets some data members.
  inline void set_flags(uint16_t flags) { m_flags = flags; }
  inline void add_flags(uint16_t flags) { m_flags |= flags; }
  inline void remove_flags(uint16_t flags) { m_flags &= ~flags; }
  inline void set_color32(uint32_t color) { m_color = color; }

  // Clear the pointers to children.
  void clear_child_ptrs();

  protected:
  // Used to type-case some pointers. (Might be removed in future.)
  inline uint8_t* pixels(uint32_t* line) {
    return (uint8_t*)line;
  }

  // Recompute the geometry and paint list membership of a primitive.
  void recompute_primitive(DiPrimitive* prim, uint16_t old_flags,
                            int32_t old_min_group, int32_t old_max_group);

  // Allocate a set of dynamic functions.
  void allocate_functions(uint32_t width);

  // Deallocate a set of dynamic functions;
  void deallocate_functions();

  // Get an index to one of the dynamic functions, based on an X coordinate.
  int32_t get_function_index(int32_t width, int32_t x, int32_t view_x_extent);

  // Set a pointer to one of the dynamic functions, based on an X coordinate.
  void set_current_function(int32_t width, int32_t x, int32_t view_x_extent);

  int32_t   m_view_x;       // upper-left x coordinate of the enclosing viewport, relative to the screen
  int32_t   m_view_y;       // upper-left y coordinate of the enclosing viewport, relative to the screen
  int32_t   m_view_x_extent; // lower-right x coordinate plus 1, of the enclosing viewport
  int32_t   m_view_y_extent; // lower-right y coordinate plus 1, of the enclosing viewport
  int32_t   m_rel_x;        // upper-left x coordinate, relative to the parent
  int32_t   m_rel_y;        // upper-left y coordinate, relative to the parent
  int32_t   m_rel_dx;       // auto-delta-x as a 16-bit fraction, relative to the parent
  int32_t   m_rel_dy;       // auto-delta-y as a 16-bit fraction, relative to the parent
  int32_t   m_auto_moves;   // number of times to move this primitive automatically
  int32_t   m_abs_x;        // upper-left x coordinate, relative to the screen
  int32_t   m_abs_x_word;   // m_abs_x & 0xFFFFFFFC (word boundary)
  int32_t   m_abs_y;        // upper-left y coordinate, relative to the screen
  int32_t   m_width;        // coverage width in pixels
  int32_t   m_height;       // coverage height in pixels
  int32_t   m_x_extent;     // sum of m_abs_x + m_width
  int32_t   m_y_extent;     // sum of m_abs_y + m_height
  int32_t   m_draw_x;       // max of m_abs_x and m_view_x
  int32_t   m_draw_y;       // max of m_abs_y and m_view_y
  int32_t   m_draw_x_extent; // min of m_x_extent and m_view_x_extent
  int32_t   m_draw_y_extent; // min of m_y_extent and m_view_y_extent
  int32_t   m_draw_x_offset; // difference of m_draw_x - m_abs_x
  int32_t   m_draw_y_offset; // difference of m_draw_y - m_abs_y
  int32_t   m_draw_x_word;  // m_draw_x & 0xFFFFFFFC (word boundary)
  int32_t   m_draw_x_word_offset; // difference of m_draw_x_word - m_abs_x_word
  uint32_t  m_color;        // applies to some primitives, but not to others
  uint32_t  m_future32;     // for potential future use
  DiPrimitive* m_parent;       // id of parent primitive
  DiPrimitive* m_first_child;  // id of first child primitive
  DiPrimitive* m_last_child;   // id of last child primitive
  DiPrimitive* m_prev_sibling; // id of previous sibling primitive
  DiPrimitive* m_next_sibling; // id of next sibling primitive
  int16_t   m_first_group;  // lowest index of drawing group in which it is a member
  int16_t   m_last_group;   // highest index of drawing group in which it is a member
  int16_t   m_id;           // id of this primitive
  uint16_t  m_flags;        // flag bits to control painting, etc.
};

#pragma pack(pop)