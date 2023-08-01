// di_primitive.cpp - Function definitions for base drawing primitives
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

#include "di_primitive.h"

DiPrimitive::DiPrimitive() {}

DiPrimitive::~DiPrimitive() {}

void DiPrimitive::get_vertical_line_range(int32_t* min_y, int32_t* max_y) {
  *min_y = 0;
  *max_y = 0;
}

void DiPrimitive::get_vertical_group_range(int32_t* min_group, int32_t* max_group) {
  int32_t min_y, max_y;
  get_vertical_line_range(&min_y, &max_y);
  min_y = MAX(min_y, 0);
  max_y = MIN(max_y, (ACT_LINES-1));
  *min_group = min_y >> VERTICAL_GROUP_INDEX_SHIFT;
  *max_group = max_y >> VERTICAL_GROUP_INDEX_SHIFT;
}

void IRAM_ATTR DiPrimitive::paint(const DiPaintParams *params) {}

DiPrimitiveX::DiPrimitiveX(): m_x(0) {}

DiPrimitiveX::DiPrimitiveX(int32_t x): m_x(x) {}

DiPrimitiveXC::DiPrimitiveXC(): m_color(0) {}

DiPrimitiveXC::DiPrimitiveXC(int32_t x, uint8_t color):
  DiPrimitiveX(x), m_color(color) {}

DiPrimitiveY::DiPrimitiveY(): m_y(0) {}

DiPrimitiveY::DiPrimitiveY(int32_t y): m_y(y) {}

void DiPrimitiveY::get_vertical_line_range(int32_t* min_y, int32_t* max_y) {
  *min_y = m_y;
  *max_y = m_y;
}

DiPrimitiveYC::DiPrimitiveYC(): m_color(0) {}

DiPrimitiveYC::DiPrimitiveYC(int32_t y, uint8_t color):
  DiPrimitiveY(y), m_color(color) {}

DiPrimitiveXY::DiPrimitiveXY(): m_y(0) {}

DiPrimitiveXY::DiPrimitiveXY(int32_t x, int32_t y):
  DiPrimitiveX(x), m_y(y) {}

void DiPrimitiveXY::get_vertical_line_range(int32_t* min_y, int32_t* max_y) {
  *min_y = m_y;
  *max_y = m_y;
}

DiPrimitiveXYC::DiPrimitiveXYC(): m_color(0) {}

DiPrimitiveXYC::DiPrimitiveXYC(int32_t x, int32_t y, uint8_t color):
  DiPrimitiveXY(x, y), m_color(color) {}

DiPrimitiveXYW::DiPrimitiveXYW(): m_width(0), m_x_extent(0) {}

DiPrimitiveXYW::DiPrimitiveXYW(int32_t x, int32_t y, int32_t width):
  DiPrimitiveXY(x, y), m_width(width), m_x_extent(x+width) {}

DiPrimitiveXYWC::DiPrimitiveXYWC(): m_color(0) {}

DiPrimitiveXYWC::DiPrimitiveXYWC(int32_t x, int32_t y, int32_t width, uint8_t color):
  DiPrimitiveXYW(x, y, width), m_color(color) {}

DiPrimitiveXYH::DiPrimitiveXYH(): m_height(0), m_y_extent(0) {}

DiPrimitiveXYH::DiPrimitiveXYH(int32_t x, int32_t y, int32_t height):
  DiPrimitiveXY(x, y), m_height(height), m_y_extent(y+height) {}

void DiPrimitiveXYH::get_vertical_line_range(int32_t* min_y, int32_t* max_y) {
  *min_y = m_y;
  *max_y = m_y_extent - 1;
}

DiPrimitiveXYHC::DiPrimitiveXYHC(): m_color(0) {}

DiPrimitiveXYHC::DiPrimitiveXYHC(int32_t x, int32_t y, int32_t height, uint8_t color):
  DiPrimitiveXYH(x, y, height), m_color(color) {}

DiPrimitiveXYWH::DiPrimitiveXYWH(): m_height(0), m_y_extent(0) {}

DiPrimitiveXYWH::DiPrimitiveXYWH(int32_t x, int32_t y, int32_t width, int32_t height):
  DiPrimitiveXYW(x, y, width), m_height(height), m_y_extent(y+height) {}

void DiPrimitiveXYWH::get_vertical_line_range(int32_t* min_y, int32_t* max_y) {
  *min_y = m_y;
  *max_y = m_y_extent - 1;
}

DiPrimitiveXYWHC::DiPrimitiveXYWHC(): m_color(0) {}

DiPrimitiveXYWHC::DiPrimitiveXYWHC(int32_t x, int32_t y, int32_t width, int32_t height, uint8_t color):
  DiPrimitiveXYWH(x, y, width, height), m_color(color) {}
