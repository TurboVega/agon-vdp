// di_general_line.h - Function declarations for drawing general lines
//
// A general line is 1 pixel thick, and connects any 2 points, except that
// it should not be used for vertical, horizontal, or precisely diagonal
// lines, because there are other optimized classes for those cases.
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
#include "di_primitive.h"
#include "di_line_pieces.h"
#include "di_code.h"

class DiGeneralLine: public DiPrimitive {
  public:
  DiLinePieces  m_line_pieces; // determines how pixels on each scan line are written
  uint8_t       m_opaqueness;

  // Construct a general line. This requires calling init_params() afterward.
  DiGeneralLine();

  // This function constructs a line from two points. The upper 2 bits of
  // the color must be zeros.
  void make_line(uint16_t flags, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color, uint8_t opaqueness);

  // This function constructs a triangle outline from three points. The
  // upper 2 bits of the color must be zeros.
  void make_triangle_outline(uint16_t flags, int32_t x1, int32_t y1, int32_t x2,
            int32_t y2, int32_t x3, int32_t y3, uint8_t color, uint8_t opaqueness);

  // This function constructs a solid (filled) triangle from three points. The
  // upper 2 bits of the color must be zeros.
  void make_solid_triangle(uint16_t flags, int32_t x1, int32_t y1, int32_t x2,
            int32_t y2, int32_t x3, int32_t y3, uint8_t color, uint8_t opaqueness);

  // Clear the custom instructions needed to draw the primitive.
  virtual void IRAM_ATTR delete_instructions();
   
  // Reassemble the custom instructions needed to draw the primitive.
  virtual void IRAM_ATTR generate_instructions();
   
  virtual void IRAM_ATTR paint(volatile uint32_t* p_scan_line, uint32_t line_index);

  protected:
  EspFunction* m_paint_fcn;

  void create_functions();
};