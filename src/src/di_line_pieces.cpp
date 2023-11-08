// di_line_points.cpp - Function definitions for generating line points
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

#include "di_line_pieces.h"
#include <cstddef>
#include <string.h>

typedef union {
  int64_t value64;
  struct {
    uint32_t low;
    int32_t high;
  } value32;
} Overlay;

void DiLineSections::add_piece(int16_t x, uint16_t width, bool solid) {
  auto x_extent = x + width;
  for (auto piece = m_pieces.begin(); piece != m_pieces.end(); piece++) {
    auto piece_extent = piece->m_x + piece->m_width;
    // (a)  px----------pe
    //            x-----------xe
    //
    // (b)        px----------pe
    //       x-----------xe
    //
    // (c)       x-------xe
    //              px--------pe
    // (d)   x-------xe
    //     px--------pe
    if (solid ||
        x >= piece->m_x && x <= piece_extent || // case (a)
        x_extent >= piece->m_x && x_extent <= piece_extent || // case (b)
        piece->m_x >= x && piece->m_x <= x_extent || // case (c)
        piece_extent >= x && piece_extent <= x_extent) { // case (d)
      // merge the new piece with the old piece
      piece->m_x = MIN(piece->m_x, x);
      auto extent = MAX(x_extent, piece_extent);
      piece->m_width = extent - piece->m_x;
      return;
    } else if (x_extent < piece->m_x) {
      // insert a new piece before the old piece
      DiLinePiece new_piece;
      new_piece.m_x = x;
      new_piece.m_width = width;
      m_pieces.insert(piece, new_piece);
      return;
    }
  }
  // insert a new piece after the old piece
  DiLinePiece new_piece;
  new_piece.m_x = x;
  new_piece.m_width = width;
  m_pieces.push_back(new_piece);
}

DiLineDetails::DiLineDetails() {
  m_min_x = 0;
  m_min_y = 0;
  m_max_x = 0;
  m_max_y = 0;
}

DiLineDetails::~DiLineDetails() {
}

void DiLineDetails::make_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, bool solid) {

}

void DiLineDetails::make_triangle_outline(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
  make_line(x1, y1, x2, y2, false);
  make_line(x2, y2, x3, y3, false);
  make_line(x3, y3, x1, y1, false);
}

void DiLineDetails::make_solid_triangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
  make_line(x1, y1, x2, y2, true);
  make_line(x2, y2, x3, y3, true);
  make_line(x3, y3, x1, y1, true);
}

void DiLineDetails::add_piece(int16_t x, int16_t y, uint16_t width) {
  if (m_sections.size()) {
    // determine whether to add a new section
    if (y < m_min_y) {
      // insert one or more new sections at lower Y values
      auto new_count = m_min_y - y;
      DiLineSections new_sections;
      m_sections.insert(m_sections.begin(), new_count, new_sections);
      m_sections[0].add_piece(x, width);
    } else if (y > m_max_y) {
      // insert one or more new sections at higher Y values
      auto new_count = y - m_max_y;
      DiLineSections new_sections;
      m_sections.resize(m_sections.size() + new_count);
      m_sections[m_sections.size() - 1].add_piece(x, width);
    } else {
      // reuse an existing section with the same Y value
      m_sections[y - m_min_y].add_piece(x, width);
    }
  } else {
    // add the first section
    DiLineSections new_section;
    new_section.add_piece(x, width);
    m_min_x = x;
    m_min_y = y;
    m_max_x = x;
    m_max_y = y;
  }
}

/*
void DiLineDetails::make_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
  m_min_x = MIN(x1, x2);
  m_max_x = MAX(x1, x2);
  m_min_y = MIN(y1, y2);
  m_max_y = MAX(y1, y2);

  int16_t dx = m_max_x - m_min_x;
  int16_t dy = m_max_y - m_min_y;
  int16_t delta = MAX(dx, dy);

  if (!delta) {
    DiLinePiece piece;
    piece.m_x = x1;
    piece.m_y = y1;
    piece.m_width = 1;
    m_sections.push_back(piece);
    return;
  }
  
  Overlay x;
  x.value32.low = 0;
  x.value32.high = m_min_x;
  int64_t delta_x = (((int64_t)dx) << 32) / delta + 1;

  Overlay y;
  y.value32.low = 0;
  y.value32.high = m_min_y;
  int64_t delta_y = (((int64_t)dy) << 32) / delta + 1;

  m_pieces = new DiLinePiece[delta+1];
  int32_t first_x = x.value32.high;
  int32_t first_y = y.value32.high;
  uint16_t i = 0;

  bool x_at_end = (x1 == x2);
  bool y_at_end = (y1 == y2);

  while (i < delta) {
    Overlay nx;
    Overlay ny;
    if (!x_at_end) {
      nx.value64 = x.value64 + delta_x;
      if (nx.value32.high == m_max_x) {
        x_at_end = true;
      }
    } else {
      nx.value32.high = first_x;
      nx.value32.low = 0;
    }
    
    if (!y_at_end) {
      ny.value64 = y.value64 + delta_y;
      if (ny.value32.high == m_max_y) {
        y_at_end = true;
      }
    } else {
      ny.value32.high = first_y;
      ny.value32.low = 0;
    }

    if (/ *nx.value32.high != first_x ||* / ny.value32.high != first_y) {
      m_pieces[i].m_x = (int16_t)first_x;
      m_pieces[i].m_y = (int16_t)first_y;
      uint16_t width = (uint16_t)(ABS(nx.value32.high - first_x));
      if (width == 0) {
          width = 1;
      }
      m_pieces[i++].m_width = width;
      
      first_x = nx.value32.high;
      first_y = ny.value32.high;
    }

    if (x_at_end && y_at_end) {
      break;
    }

    x.value64 += delta_x;
    y.value64 += delta_y;
  }

  uint16_t width = (int16_t)(ABS(m_max_x - first_x + 1));
  m_pieces[i].m_x = (int16_t)first_x;
  m_pieces[i].m_y = (int16_t)first_y;
  if (width == 0) {
      width = 1;
  }
  m_pieces[i++].m_width = width;
  m_num_pieces = i;
  
  if (x1<x2 && y1>y2) {
    // Flip the line vertically
    uint16_t mid = i/2;
    for (uint16_t j = 0; j < mid; j++) {
      DiLinePiece* pj = &m_pieces[j];
      DiLinePiece* pt = &m_pieces[i - 1 - j];
      uint16_t y = pj->m_y;
      pj->m_y = pt->m_y;
      pt->m_y = y;
    }
  } else if (x1>x2 && y1<y2) {
    uint16_t mid = i/2;
    for (uint16_t j = 0; j < mid; j++) {
      DiLinePiece* pj = &m_pieces[j];
      DiLinePiece* pt = &m_pieces[i - 1 - j];
      uint16_t x = pj->m_x;
      pj->m_x = pt->m_x;
      pt->m_x = x;
      uint16_t w = pj->m_width;
      pj->m_width = pt->m_width;
      pt->m_width = w;
    }
  }
}
extern void debug_log(const char* fmt, ...);
void DiLineDetails::make_triangle_outline(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
  DiLineDetails lp[3];
  lp[0].make_line(x1, y1, x2, y2);
  lp[1].make_line(x2, y2, x3, y3);
  lp[2].make_line(x3, y3, x1, y1);

  auto min_x = MIN(x1, x2);
  auto max_x = MAX(x1, x2);
  auto min_y = MIN(y1, y2);
  auto max_y = MAX(y1, y2);
  m_min_x = MIN(min_x, x3);
  m_max_x = MAX(max_x, x3);
  m_min_y = MIN(min_y, y3);
  m_max_y = MAX(max_y, y3);

  // An outline has separate pieces for left and right lines.
  auto num_pieces = m_max_y - m_min_y + 1;
  m_num_pieces = num_pieces * 2;
  m_pieces = new DiLinePiece[num_pieces];
  debug_log("np %u, mnp %u\n", num_pieces, m_num_pieces);

  uint16_t imp = 0;
  for (uint16_t i = 0; i < num_pieces; i++) {
    auto merge_piece = &m_pieces[imp++];
    merge_piece->m_y = m_min_y + i;
    merge_piece->m_flags = 0;
    merge_piece = &m_pieces[imp++];
    merge_piece->m_y = m_min_y + i;
    merge_piece->m_flags = 0;
  }

  for (uint16_t line = 0; line < 3; line++) {
    auto lpn = &lp[line];

    debug_log("\n");
    for (uint16_t i = 0; i < lpn->m_num_pieces; i++) {
      auto piece = &lpn->m_pieces[i];
      debug_log("lp[%hu] i %hu, y %hu x %hu w %hu\n", line, i, piece->m_y, piece->m_x, piece->m_width);
    }
    debug_log("\n");

    for (uint16_t i = 0; i < lpn->m_num_pieces; i++) {
      auto line_piece = &lpn->m_pieces[i];
      auto merge_index = (line_piece->m_y - m_min_y) * 2;
      auto merge_piece = &m_pieces[merge_index];
      if (merge_piece->m_flags) {
        // This is the second piece at this Y position.
        // We must order them for left vs right.
        if (merge_piece->m_x < line_piece->m_x ||
            (merge_piece->m_x == line_piece->m_x &&
             merge_piece->m_width < line_piece->m_width)) {
          // Already in good order. Set the second piece in the pair.
          merge_piece++;
          merge_piece->m_x = line_piece->m_x;
          merge_piece->m_width = line_piece->m_width;
        } else {
          // Order needs to be reversed, so move the first piece.
          auto second_piece = merge_piece + 1;
          second_piece->m_x = merge_piece->m_x;
          second_piece->m_width = merge_piece->m_width;
          // Set the first piece.
          merge_piece->m_x = line_piece->m_x;
          merge_piece->m_width = line_piece->m_width;
        }
      } else {
        // Both pieces in the pair are empty, so set one of them.
        merge_piece->m_x = line_piece->m_x;
        merge_piece->m_width = line_piece->m_width;
        merge_piece->m_flags = 1;
      }
    }
  }

  for (uint16_t i = 0; i < m_num_pieces; i++) {
    auto piece = &m_pieces[i];
    debug_log("i %hu, left x %hu w %hu", i, piece->m_x, piece->m_width);
    piece = &m_pieces[++i];
    debug_log(", right x %hu w %hu\n", piece->m_x, piece->m_width);
  }
}

void DiLineDetails::make_solid_triangle(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t x3, int16_t y3) {
  DiLineDetails lp[3];
  lp[0].make_line(x1, y1, x2, y2);
  lp[1].make_line(x2, y2, x3, y3);
  lp[2].make_line(x3, y3, x1, y1);

  int16_t min_x = MIN(x1, x2);
  int16_t max_x = MAX(x1, x2);
  int16_t min_y = MIN(y1, y2);
  int16_t max_y = MAX(y1, y2);
  m_min_x = MIN(min_x, x3);
  m_max_x = MAX(max_x, x3);
  m_min_y = MIN(min_y, y3);
  m_max_y = MAX(max_y, y3);

  m_num_pieces = m_max_y - m_min_y + 1;
  m_pieces = new DiLinePiece[m_num_pieces];

  for (uint16_t i = 0; i < m_num_pieces; i++) {
    DiLinePiece* merge_piece = &m_pieces[i];
    merge_piece->m_y = m_min_y + i;
    merge_piece->m_flags = 0;
  }

  for (uint16_t line = 0; line < 3; line++) {
    DiLineDetails* lpn = &lp[line];
    for (uint16_t i = 0; i < lpn->m_num_pieces; i++) {
      DiLinePiece* line_piece = &lpn->m_pieces[i];
      uint16_t merge_index = line_piece->m_y - m_min_y;
      DiLinePiece* merge_piece = &m_pieces[merge_index];
      if (merge_piece->m_flags) {
        uint16_t left = MIN(line_piece->m_x, merge_piece->m_x);
        uint16_t right1 = line_piece->m_x + line_piece->m_width - 1;
        uint16_t right2 = merge_piece->m_x + merge_piece->m_width - 1;
        uint16_t right = MAX(right1, right2);
        merge_piece->m_x = left;
        merge_piece->m_width = right - left + 1;
      } else {
        merge_piece->m_x = line_piece->m_x;
        merge_piece->m_width = line_piece->m_width;
        merge_piece->m_flags = 1;
      }
    }
  }
}
*/
