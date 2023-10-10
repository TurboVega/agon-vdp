// di_terminal.cpp - Function definitions for supporting a character terminal display
//
// A terminal is a specialized tile map, where each tile is a single character
// cell, and the character codes are used as tile image IDs.
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

#include "di_terminal.h"
#include <cstring>

DiTerminal::DiTerminal(uint32_t x, uint32_t y, uint8_t flags, uint32_t codes,
                        uint32_t columns, uint32_t rows,
                        uint8_t fg_color, uint8_t bg_color, const uint8_t* font) :
  DiTileMap(ACT_PIXELS, ACT_LINES, columns, rows, 8, 8, flags) {
  m_current_column = 0;
  m_current_row = 0;

  // Copy built-in font pixel data to the bitmaps for this terminal.
  for (int b = 0; b < codes; b++) {
    auto bm_id = (DiTileBitmapID)b;
    create_bitmap(bm_id);
    uint32_t char_start = (uint32_t)b * 8;
    for (int y = 0; y < 8; y++) {
      uint8_t pixels = font[char_start+y];
      for (int x = 0; x < 8; x++) {
        if (pixels & 0x80) {
          set_pixel(bm_id, x, y, fg_color);
        } else {
          set_pixel(bm_id, x, y, bg_color);
        }
        pixels <<= 1;
      }
    }
  }

  clear_screen();
}

DiTerminal::~DiTerminal() {
}

void DiTerminal::set_character_position(int32_t column, int32_t row) {
  m_current_column = column;
  m_current_row = row;
}

void DiTerminal::write_character(uint8_t character) {
  if (m_current_column < 0) {
    // scroll text to the right (insert on the left)
    int32_t move = m_columns + m_current_column;
    int32_t open = -m_current_column;
    move_text(0, 0, move, m_rows, open, 0);
    erase_text(0, 0, open, m_rows);
    m_current_column = 0;
  }
  if (m_current_column >= m_columns) {
    // scroll text to the left (insert on the right)
    int32_t open = m_current_column - m_columns + 1;
    int32_t move = m_columns - open;
    move_text(open, 0, move, m_rows, 0, 0);
    erase_text(move, 0, open, m_rows);
    m_current_column = m_columns - 1;
  }
  if (m_current_row < 0) {
    // scroll text down (insert at the top)
    int32_t move = m_rows + m_current_row;
    int32_t open = -m_current_row;
    move_text(0, 0, m_columns, move, 0, open);
    erase_text(0, 0, m_columns, open);
    m_current_row = 0;
  }
  if (m_current_row >= m_rows) {
    // scroll text up (insert at the bottom)
    int32_t open = m_current_row - m_rows + 1;
    int32_t move = m_rows -  open;
    move_text(0, open, m_columns, move, 0, -open);
    erase_text(0, move, m_columns, open);
    m_current_row = m_rows - 1;
  }

  // Set the tile image ID using the character code.
  set_tile(m_current_column, m_current_row, (DiTileBitmapID)character);

  // Advance the current position
  if (++m_current_column >= m_columns) {
    m_current_column = 0;
    m_current_row++;
  }
}

void DiTerminal::write_character(int32_t column, int32_t row, uint8_t character) {
  set_tile(column, row, character);
}

uint8_t DiTerminal::read_character() {
  return read_character(m_current_column, m_current_row);
}

uint8_t DiTerminal::read_character(int32_t column, int32_t row) {
  return get_tile(column, row);
}

void DiTerminal::erase_text(int32_t column, int32_t row, int32_t columns, int32_t rows) {
  while (rows-- > 0) {
    int32_t col = column;
    for (int32_t c = 0; c < columns; c++) {
      write_character(col++, row, 0x20);
    }
    row++;
  }
}

void DiTerminal::move_text(int32_t column, int32_t row, int32_t columns, int32_t rows,
                            int32_t delta_horiz, int32_t delta_vert) {
  if (delta_vert > 0) {
    // moving rows down; copy bottom-up
    row += rows - 1;
    while (rows-- > 0) {
      auto col = column;
      auto n = columns;
      while (n > 0) {
        auto ch = get_tile(col, row);
        set_tile(col++, row, ch);
      }
      row--;
    }
  } else {
    // moving rows up; copy top-down
    while (rows-- > 0) {
      auto col = column;
      auto n = columns;
      while (n > 0) {
        auto ch = get_tile(col, row);
        set_tile(col++, row, ch);
      }
      row++;
    }
  }
}

void DiTerminal::clear_screen() {
  erase_text(0, 0, m_columns, m_rows);
  m_current_column = 0;
  m_current_row = 0;
}

void DiTerminal::move_cursor_left() {
  if (m_current_column > 0) {
    m_current_column--;
  } else if (m_current_row > 0) {
    m_current_row--;
    m_current_column = m_columns - 1;
  }
}

void DiTerminal::move_cursor_right() {
  if (m_current_column < m_columns - 1) {
    m_current_column++;
  } else if (m_current_row < m_rows - 1) {
    m_current_row++;
    m_current_column = 0;
  }
}

void DiTerminal::move_cursor_down() {
  m_current_row++;
}

void DiTerminal::move_cursor_up() {
   if (m_current_row > 0) {
    m_current_row--;
  }
}

void DiTerminal::move_cursor_home() {
  m_current_row = 0;
  m_current_column = 0;
}

void DiTerminal::move_cursor_boln() {
  m_current_column = 0;
}

void DiTerminal::do_backspace() {
  if (m_current_column > 0) {
    m_current_column--;
    write_character(m_current_column, m_current_row, 0x20);
  } else if (m_current_row > 0) {
    m_current_row--;
    m_current_column = m_columns - 1;
    write_character(m_current_column, m_current_row, 0x20);
  }
}

void DiTerminal::move_cursor_tab(uint8_t x, uint8_t y) {
  set_character_position(x, y);
}

void DiTerminal::get_position(uint16_t& column, uint16_t& row) {
  column = m_current_column;
  row = m_current_row;
}