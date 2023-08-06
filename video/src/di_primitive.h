// primitive.h - Function declarations for base drawing primitives
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

// Used as a hint, to potentially save RAM, when allocating bitmaps.
typedef enum ScrollMode {
  NONE,       // do not allow scrolling
  HORIZONTAL, // allow horizontal, but not vertical
  VERTICAL,   // allow vertical, but not horizontal
  BOTH        // both horizontal and vertical
};

#pragma pack(push,1)

// Used to paint the primitives that belong to vertical groups of scan lines.
typedef struct {
  volatile uint32_t* m_line32;  // address of the DMA visible data
  volatile uint8_t*  m_line8;   // address of the DMA visible data
  int32_t   m_line_index;       // scan line index on the screen (0=top, 599=bottom)
  int32_t   m_scrolled_y;       // sum of m_line_index + m_vert_scroll
  int32_t   m_horiz_scroll;     // used to move all top-level primitives in the X direction
  int32_t   m_vert_scroll;      // used to move all top-level primitives in the Y direction
  int32_t   m_screen_width;     // fixed at 800
  int32_t   m_screen_height;    // fixed at 600
} DiPaintParams;

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

  // Gets the range of Y scan lines used by the primitive.
  void IRAM_ATTR get_vertical_line_range(int32_t& min_y, int32_t& max_y);

  // Draws the primitive to the DMA scan line buffer.
  virtual void IRAM_ATTR paint(const DiPaintParams *params);

  // Groups scan lines for optimizing paint calls.
  void IRAM_ATTR get_vertical_group_range(int32_t& min_group, int32_t& max_group);

  // Attach a child primitive.
  void IRAM_ATTR attach_child(DiPrimitive* child);

  // Detach a child primitive.
  void IRAM_ATTR detach_child(DiPrimitive* child);

  // Set the X, Y position relative to the parent.
  void IRAM_ATTR set_relative_position(int32_t rel_x, int32_t rel_y);

  // Set the delta X, Y position, relative to the parent, and the move count.
  // These values are used to update the relative position automatically, frame-by-frame.
  void IRAM_ATTR set_relative_deltas(int32_t rel_dx, int32_t rel_dy, uint32_t auto_moves);

  // Compute the absolute position and related data members, based on the
  // current position, relative to the parent primitive. The viewport of
  // this primitive is based on the given viewport parameters and certain flags.
  void IRAM_ATTR compute_absolute_geometry(int32_t view_x, int32_t view_y, int32_t view_x_extent, int32_t view_y_extent);

  // Gets various data members.
  inline uint16_t get_id() { return m_id; }
  inline uint8_t get_flags() { return m_flags; }
  inline int32_t get_view_x() { return m_view_x; }
  inline int32_t get_view_y() { return m_view_y; }
  inline int32_t get_view_x_extent() { return m_view_x_extent; }
  inline int32_t get_view_y_extent() { return m_view_y_extent; }
  inline DiPrimitive* get_parent() { return m_parent; }
  inline DiPrimitive* get_first_child() { return m_first_child; }
  inline DiPrimitive* get_next_sibling() { return m_next_sibling; }

  // Clear the pointers to children.
  void clear_child_ptrs();

  protected:
  // Used to type-case some pointers. (Might be removed in future.)
  inline uint8_t* pixels(uint32_t* line) {
    return (uint8_t*)line;
  }

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
  int32_t   m_abs_y;        // upper-left y coordinate, relative to the screen
  int32_t   m_width;        // coverage width in pixels
  int32_t   m_height;       // coverage height in pixels
  int32_t   m_x_extent;     // sum of m_abs_x + m_width
  int32_t   m_y_extent;     // sum of m_abs_y + m_height
  DiPrimitive* m_parent;       // id of parent primitive
  DiPrimitive* m_first_child;  // id of first child primitive
  DiPrimitive* m_last_child;   // id of last child primitive
  DiPrimitive* m_prev_sibling; // id of previous sibling primitive
  DiPrimitive* m_next_sibling; // id of next sibling primitive
  int16_t   m_first_group;  // lowest index of drawing group in which it is a member
  int16_t   m_last_group;   // highest index of drawing group in which it is a member
  int16_t   m_id;           // id of this primitive
  uint8_t   m_flags;        // flag bits to control painting, etc.
  uint8_t   m_color;        // applies to some primitives, but not to others
};

#define PRIM_FLAG_PAINT_THIS  0x01  // whether to paint this primitive
#define PRIM_FLAG_PAINT_KIDS  0x02  // whether to paint child primitives
#define PRIM_FLAG_CLIP_THIS   0x04  // whether to clip this primitive
#define PRIM_FLAG_CLIP_KIDS   0x08  // whether to clip child primitives

#pragma pack(pop)
