// di_line_pieces.h - Function declarations for generating line pieces
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
#include "di_constants.h"

#pragma pack(push,2)

// This structure tells how to draw on a single scan line,
// and represents a (possibly) short section of a larger line.
typedef struct {
  int16_t m_x;
  int16_t m_y;
  uint16_t m_width;
  uint16_t m_flags;
} DiLinePiece;

#pragma pack(pop)

class DiLinePieces {
  public:
  DiLinePiece*  m_pieces;
  int16_t       m_min_x;
  int16_t       m_min_y;
  int16_t       m_max_x;
  int16_t       m_max_y;
  uint16_t      m_num_pieces;

  // Constructs an empty object. You must call a function below to create the line pieces.
  DiLinePieces();

  // Destroys the line pieces.
  ~DiLinePieces();

  // This function creates line pieces for a line from two points.
  void generate_line_pieces(int16_t x1, int16_t y1, int16_t x2, int16_t y2);

  // This function creates a solid (filled) triangle from three points.
  void generate_line_pieces(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3);
};
