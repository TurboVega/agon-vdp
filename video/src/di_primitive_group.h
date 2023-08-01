// di_primitive_group.h - Function declarations for grouping primitives
//
// A primitive group is a group of display primtives that can be shown,
// hidden, or moved as a unit (as a collection).
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
#include <vector>

class DiPrimitiveGroup: public DiPrimitive {
  public:
  // Constructs a group ready to add new primitives.
  DiPrimitiveGroup();

  // Destroys a group and its child primitives.
  virtual ~DiPrimitiveGroup();

  // Computes the vertical Y coordinate range of the entire group.
  virtual void get_vertical_line_range(int32_t* min_y, int32_t* max_y);

  virtual void IRAM_ATTR paint(const DiPaintParams *params);

  // Adds a primitive to the group. This can be a child group.
  void add_primitive(DiPrimitive* prim);

  // Removes a primitive from the group. This can be a child group.
  void remove_primitive(DiPrimitive* prim);

  // Relocates the entire group, relative to (0, 0), which is the top-left of the screen.
  // Note that the (X,Y) positions of child primitives are relative to the group, not to
  // the screen, so moving the group will move them all, on the screen.
  void set_offsets(int32_t delta_x, int32_t delta_y);

  protected:
  int32_t m_delta_x;
  int32_t m_delta_y;
  std::vector<DiPrimitive*> m_children;
};
