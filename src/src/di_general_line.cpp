// di_general_line.cpp - Function definitions for drawing general lines
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

#include "di_general_line.h"

static int32_t min3(int32_t a, int32_t b, int32_t c) {
  int32_t m = MIN(a, b);
  return MIN(m, c);
}

static int32_t max3(int32_t a, int32_t b, int32_t c) {
  int32_t m = MAX(a, b);
  return MAX(m, c);
}

static int16_t min_of_pairs(const int16_t* coords, uint16_t n) {
  int16_t m = *coords;
  while (--n) {
    coords += 2;
    auto c = *coords;
    m = MIN(m, c);
  }
  return m;
}

static int16_t max_of_pairs(const int16_t* coords, uint16_t n) {
  int16_t m = *coords;
  while (--n) {
    coords += 2;
    auto c = *coords;
    m = MAX(m, c);
  }
  return m;
}

DiGeneralLine::DiGeneralLine() {}

void DiGeneralLine::make_line(uint16_t flags, int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                uint8_t color, uint8_t opaqueness) {
  int16_t coords[4];
  coords[0] = x1;
  coords[1] = y1;
  coords[2] = x2;
  coords[3] = y2;
  make_line(flags, coords, color, opaqueness);
}

void DiGeneralLine::make_line(uint16_t flags, int16_t* coords, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, 2, color, opaqueness);
  m_line_details.make_line(1, coords[0], coords[1], coords[2], coords[3], false);
  create_functions();
}

void DiGeneralLine::make_triangle_outline(uint16_t flags, int16_t* coords, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, 3, color, opaqueness);
  m_line_details.make_triangle_outline(1, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);
  create_functions();
}

void DiGeneralLine::make_solid_triangle(uint16_t flags, int16_t* coords, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, 3, color, opaqueness);
  m_line_details.make_solid_triangle(1, coords[0], coords[1], coords[2], coords[3], coords[4], coords[5]);
  create_functions();
}
extern void debug_log(const char* fmt, ...);

void DiGeneralLine::init_from_coords(uint16_t flags, int16_t* coords,
          uint16_t n, uint8_t color, uint8_t opaqueness) {
  m_flags = flags;
  m_opaqueness = opaqueness;
  m_rel_x = min_of_pairs(coords, n);
  m_rel_y = min_of_pairs(coords+1, n);
  m_width = max_of_pairs(coords, n) - m_rel_x + 1;
  m_height = max_of_pairs(coords+1, n) - m_rel_y + 1;
  //debug_log("%hi %hi, w %hi, h %i\n", m_rel_x, m_rel_y, m_width, m_height);
  color &= 0x3F; // remove any alpha bits
  m_color = PIXEL_COLOR_X4(color);

  while (n--) {
    *coords++ -= m_rel_x;
    *coords++ -= m_rel_y;
  }
}

void DiGeneralLine::make_triangle_list_outline(uint16_t flags, int16_t* coords,
          uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, n*3, color, opaqueness);

  uint8_t id = 1;
  while (n--) {
    debug_log("tri %hi %hi %hi %hi %hi %hi\n", coords[0], coords[1],
      coords[2], coords[3], coords[4], coords[5]);
    m_line_details.make_triangle_outline(id++, coords[0], coords[1],
      coords[2], coords[3], coords[4], coords[5]);
    coords += 6;
  }
  create_functions();
  debug_log("prim x %i y %i w %u h %u ld %u\n", m_rel_x, m_rel_y, m_width, m_height, m_line_details.m_sections.size());
}

void DiGeneralLine::make_solid_triangle_list(uint16_t flags, int16_t* coords,
          uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, n*3, color, opaqueness);

  uint8_t id = 1;
  while (n--) {
    DiLineDetails details;
    details.make_solid_triangle(id++, coords[0], coords[1],
      coords[2], coords[3], coords[4], coords[5]);
    coords += 6;
    m_line_details.merge(details);
  }
  create_functions();
}

void DiGeneralLine::make_triangle_fan_outline(uint16_t flags,
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {

  debug_log("init (%hu) %hi %hi %hi %hi\n", n, coords[0], coords[1], coords[2], coords[3]);
  for (uint16_t i = 4; i<n*2+4; i+=2) {
    debug_log(" %hi %hi\n", coords[i], coords[i+1]);
  }
  init_from_coords(flags, coords, n+2, color, opaqueness);

  
  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;

  uint8_t id = 1;
  while (n--) {
    debug_log("fan %hi %hi %hi %hi %hi %hi\n",sx0, sy0, sx1, sy1, coords[0], coords[1]);
    m_line_details.make_triangle_outline(id++, sx0, sy0, sx1, sy1, coords[0], coords[1]);
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 2;
  }
  create_functions();
}

void DiGeneralLine::make_solid_triangle_fan(uint16_t flags,
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, n+2, color, opaqueness);

  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;
  
  uint8_t id = 1;
  while (n--) {
    DiLineDetails details;
    details.make_solid_triangle(id++, sx0, sy0, sx1, sy1, coords[0], coords[1]);
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 2;
    m_line_details.merge(details);
  }
  create_functions();
}

void DiGeneralLine::make_triangle_strip_outline(uint16_t flags,
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, n+2, color, opaqueness);

  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;
  
  uint8_t id = 1;
  while (n--) {
    m_line_details.make_triangle_outline(id++, sx0, sy0, sx1, sy1, coords[0], coords[1]);
    sx0 = sx1;
    sy0 = sy1;
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 2;
  }
  create_functions();
}

void DiGeneralLine::make_solid_triangle_strip(uint16_t flags,
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, n+2, color, opaqueness);

  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;
  
  uint8_t id = 1;
  while (n--) {
    DiLineDetails details;
    details.make_solid_triangle(id++, sx0, sy0, sx1, sy1, coords[0], coords[1]);
    sx0 = sx1;
    sy0 = sy1;
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 2;
    m_line_details.merge(details);
  }
  create_functions();
}

void DiGeneralLine::make_quad_outline(uint16_t flags, int16_t* coords, 
          uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, 4, color, opaqueness);
  m_line_details.make_quad_outline(1, coords[0], coords[1], coords[2], coords[3],
    coords[4], coords[5], coords[6], coords[7]);
  create_functions();
}

void DiGeneralLine::make_solid_quad(uint16_t flags, int16_t* coords,
          uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, 4, color, opaqueness);
  m_line_details.make_solid_quad(1, coords[0], coords[1], coords[2], coords[3],
    coords[4], coords[5], coords[6], coords[7]);
  create_functions();
}

void DiGeneralLine::make_quad_list_outline(uint16_t flags, int16_t* coords,
          uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, n*4, color, opaqueness);

  uint8_t id = 1;
  while (n--) {
    m_line_details.make_quad_outline(id++, coords[0], coords[1],
      coords[2], coords[3], coords[4], coords[5], coords[6], coords[7]);
    coords += 8;
  }
  create_functions();
}

void DiGeneralLine::make_solid_quad_list(uint16_t flags, int16_t* coords,
          uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, n*4, color, opaqueness);

  uint8_t id = 1;
  while (n--) {
    DiLineDetails details;
    details.make_solid_quad(id++, coords[0], coords[1],
      coords[2], coords[3], coords[4], coords[5], coords[6], coords[7]);
    coords += 8;
    m_line_details.merge(details);
  }
  create_functions();
}

void DiGeneralLine::make_quad_strip_outline(uint16_t flags,
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, n*2+2, color, opaqueness);

  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;

  uint8_t id = 1;
  while (n--) {
    m_line_details.make_quad_outline(id++, sx0, sy0, sx1, sy1, coords[0], coords[1], coords[2], coords[3]);
    sx0 = coords[2];
    sy0 = coords[3];
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 4;
  }
  create_functions();
}

void DiGeneralLine::make_solid_quad_strip(uint16_t flags,
          int16_t* coords, uint16_t n, uint8_t color, uint8_t opaqueness) {
  init_from_coords(flags, coords, n*2+2, color, opaqueness);

  auto sx0 = coords[0];
  auto sy0 = coords[1];
  auto sx1 = coords[2];
  auto sy1 = coords[3];
  coords += 4;

  uint8_t id = 1;
  while (n--) {
    DiLineDetails details;
    details.make_solid_quad(id++, sx0, sy0, sx1, sy1, coords[0], coords[1], coords[2], coords[3]);
    sx0 = coords[2];
    sy0 = coords[3];
    sx1 = coords[0];
    sy1 = coords[1];
    coords += 4;
    m_line_details.merge(details);
  }
  create_functions();
}

void IRAM_ATTR DiGeneralLine::delete_instructions() {
  if (m_flags & PRIM_FLAG_H_SCROLL_1) {
    for (uint32_t pos = 0; pos < 4; pos++) {
      m_paint_fcn[pos].clear();
    }
  } else {
    m_paint_fcn[0].clear();
  }
}
//extern void debug_log(const char* fmt, ...);
void IRAM_ATTR DiGeneralLine::generate_instructions() {
  delete_instructions();
  m_flags |= PRIM_FLAGS_X;
  if (m_flags & PRIM_FLAGS_CAN_DRAW) {
    if (m_flags & PRIM_FLAG_H_SCROLL_1) {
      for (uint32_t pos = 0; pos < 4; pos++) {
        //debug_log("\nid=%hu pos=%u code=%X %X\n", m_id, pos, &m_paint_fcn[pos], m_paint_fcn[pos].get_real_address(0));
        EspFixups fixups;
        auto num_sections = (uint32_t)m_line_details.m_sections.size();
        uint32_t at_jump_table = m_paint_fcn[pos].init_jump_table(num_sections);
        for (uint32_t i = 0; i < num_sections; i++) {
          auto sections = &m_line_details.m_sections[i];
          //debug_log(" [%i] x%hi w%hi", i, piece->m_x, piece->m_width);
          m_paint_fcn[pos].align32();
          m_paint_fcn[pos].j_to_here(at_jump_table + i * sizeof(uint32_t));
          m_paint_fcn[pos].draw_line_as_inner_fcn(fixups, pos, pos,
            sections, m_flags, m_opaqueness);
        }
        m_paint_fcn[pos].do_fixups(fixups);
        //debug_log("id=%hu pos=%u code=%X %X\n", m_id, pos, &m_paint_fcn[pos], m_paint_fcn[pos].get_real_address(0));
      }
    } else {
        //debug_log("\ngen id=%hu code=%X %X\n", m_id, m_paint_fcn, m_paint_fcn[0].get_real_address(0));
        EspFixups fixups;
        auto num_sections = (uint32_t)m_line_details.m_sections.size();
        uint32_t at_jump_table = m_paint_fcn[0].init_jump_table(num_sections);
        for (uint32_t i = 0; i < num_sections; i++) {
          auto sections = &m_line_details.m_sections[i];
          //debug_log("\n > section [%i] ", i);
          m_paint_fcn[0].align32();
          m_paint_fcn[0].j_to_here(at_jump_table + i * sizeof(uint32_t));
          m_paint_fcn[0].draw_line_as_inner_fcn(fixups, 0, 0,
            sections, m_flags, m_opaqueness);
        }
        m_paint_fcn[0].do_fixups(fixups);
        //debug_log("    id=%hu code=%X %X\n", m_id, m_paint_fcn, m_paint_fcn[0].get_real_address(0));
      }
    }
}

void IRAM_ATTR DiGeneralLine::paint(volatile uint32_t* p_scan_line, uint32_t line_index) {
  if (m_flags & PRIM_FLAG_H_SCROLL_1) {
    m_paint_fcn[m_abs_x & 3].call_x(this, p_scan_line, line_index, m_draw_x);
  } else {
    m_paint_fcn[0].call_x(this, p_scan_line, line_index, m_draw_x);
  }
}

void DiGeneralLine::create_functions() {
  if (m_flags & PRIM_FLAG_H_SCROLL_1) {
    m_paint_fcn = new EspFunction[4];
    for (uint32_t pos = 0; pos < 4; pos++) {
      m_paint_fcn[pos].enter_and_leave_outer_function();
      //debug_log("CF id=%hu pos=%u code=%X %X\n", m_id, pos, &m_paint_fcn[pos], m_paint_fcn[pos].get_real_address(0));
    }
  } else {
    m_paint_fcn = new EspFunction;
    m_paint_fcn[0].enter_and_leave_outer_function();
    //debug_log("CF id=%hu code=%X %X\n", m_id, m_paint_fcn, m_paint_fcn[0].get_real_address(0));
  }
}