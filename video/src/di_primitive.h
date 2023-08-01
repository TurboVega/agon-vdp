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

#pragma pack(push,4)

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

  // Gets the range of Y scan lines used by the primitive.
  virtual void get_vertical_line_range(int32_t* min_y, int32_t* max_y);

  // Draws the primitive to the DMA scan line buffer.
  virtual void IRAM_ATTR paint(const DiPaintParams *params);

  // Groups scan lines for optimizing paint calls.
  void get_vertical_group_range(int32_t* min_group, int32_t* max_group);

  protected:
  // Used to type-case some pointers. (Might be removed in future.)
  inline uint8_t* pixels(uint32_t* line) {
    return (uint8_t*)line;
  }
};

class DiPrimitiveX: public DiPrimitive {
  public:
  int32_t m_x;

  DiPrimitiveX();
  DiPrimitiveX(int32_t x);
};

class DiPrimitiveXC: public DiPrimitiveX {
  public:
  uint32_t m_color;

  DiPrimitiveXC();
  DiPrimitiveXC(int32_t x, uint8_t color);
};

class DiPrimitiveY: public DiPrimitive {
  public:
  int32_t m_y;

  DiPrimitiveY();
  DiPrimitiveY(int32_t y);
  virtual void get_vertical_line_range(int32_t* min_y, int32_t* max_y);
};

class DiPrimitiveYC: public DiPrimitiveY {
  public:
  uint32_t m_color;

  DiPrimitiveYC();
  DiPrimitiveYC(int32_t y, uint8_t color);
};

class DiPrimitiveXY: public DiPrimitiveX {
  public:
  int32_t m_y;

  DiPrimitiveXY();
  DiPrimitiveXY(int32_t x, int32_t y);
  virtual void get_vertical_line_range(int32_t* min_y, int32_t* max_y);
};

class DiPrimitiveXYC: public DiPrimitiveXY {
  public:
  uint32_t m_color;

  DiPrimitiveXYC();
  DiPrimitiveXYC(int32_t x, int32_t y, uint8_t color);
};

class DiPrimitiveXYW: public DiPrimitiveXY {
  public:
  int32_t m_width;
  int32_t m_x_extent;

  DiPrimitiveXYW();
  DiPrimitiveXYW(int32_t x, int32_t y, int32_t width);
};

class DiPrimitiveXYWC: public DiPrimitiveXYW {
  public:
  uint32_t m_color;

  DiPrimitiveXYWC();
  DiPrimitiveXYWC(int32_t x, int32_t y, int32_t width, uint8_t color);
};

class DiPrimitiveXYH: public DiPrimitiveXY {
  public:
  int32_t m_height;
  int32_t m_y_extent;

  DiPrimitiveXYH();
  DiPrimitiveXYH(int32_t x, int32_t y, int32_t height);
  virtual void get_vertical_line_range(int32_t* min_y, int32_t* max_y);
};

class DiPrimitiveXYHC: public DiPrimitiveXYH {
  public:
  uint32_t m_color;

  DiPrimitiveXYHC();
  DiPrimitiveXYHC(int32_t x, int32_t y, int32_t height, uint8_t color);
};

class DiPrimitiveXYWH: public DiPrimitiveXYW {
  public:
  int32_t m_height;
  int32_t m_y_extent;

  DiPrimitiveXYWH();
  DiPrimitiveXYWH(int32_t x, int32_t y, int32_t width, int32_t height);
  virtual void get_vertical_line_range(int32_t* min_y, int32_t* max_y);
};

class DiPrimitiveXYWHC: public DiPrimitiveXYWH {
  public:
  uint32_t m_color;

  DiPrimitiveXYWHC();
  DiPrimitiveXYWHC(int32_t x, int32_t y, int32_t width, int32_t height, uint8_t color);
};

#pragma pack(pop)
