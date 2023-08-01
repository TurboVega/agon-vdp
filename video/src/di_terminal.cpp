// di_terminal.cpp - Function definitions for supporting a character terminal display
//
// A terminal is a specialized tile map, where each tile is a single character
// cell, and the character codes are used as tile bitmap indexes.
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

extern "C" {
IRAM_ATTR void DiTerminal_paint(void* this_ptr, const DiPaintParams *params);
}

DiTerminal::DiTerminal(uint32_t x, uint32_t y, uint32_t codes,
                        uint32_t columns, uint32_t rows,
                        uint8_t fg_color, uint8_t bg_color, const uint8_t* font) :
  DiTileMap(ACT_PIXELS, ACT_LINES, codes, columns, rows, 8, 8, false) {
  m_current_column = 0;
  m_current_row = 0;
  m_next_buffer_write = 0;
  m_next_buffer_read = 0;
  m_num_buffer_chars = 0;

  // Copy built-in font pixel data to the bitmaps for this terminal.
  for (int b = 0; b < 128; b++) {
    uint32_t char_start = (uint32_t)b * 8;
    for (int y = 0; y < 8; y++) {
      uint8_t pixels = font[char_start+y];
      for (int x = 0; x < 8; x++) {
        if (pixels & 0x80) {
          set_pixel(b, x, y, fg_color);
        } else {
          set_pixel(b, x, y, bg_color);
        }
        pixels <<= 1;
      }
    }
  }
}

DiTerminal::~DiTerminal() {
}

void DiTerminal::store_character(uint8_t character) {
  if (m_num_buffer_chars < INCOMING_DATA_BUFFER_SIZE) {
    m_incoming_data[m_next_buffer_write++] = character;
    if (m_next_buffer_write >= INCOMING_DATA_BUFFER_SIZE) {
      m_next_buffer_write = 0;
    }
    m_num_buffer_chars++;
  }
}

void DiTerminal::store_string(const uint8_t* string) {
  while (uint8_t character = *string++) {
    store_character(character);
  }
}

void DiTerminal::process_stored_characters() {
  while (m_num_buffer_chars > 0) {
    process_character(m_incoming_data[m_next_buffer_read++]);
    if (m_next_buffer_read >= INCOMING_DATA_BUFFER_SIZE) {
      m_next_buffer_read = 0;
    }
    m_num_buffer_chars--;
  }
}

void DiTerminal::set_position(int32_t column, int32_t row) {
  m_current_column = column;
  m_current_row = row;
}

void DiTerminal::process_character(int8_t character) {
  if (character >= 0x20 && character != 0x7F) {
    // printable character
    write_character(character);
  } else {
    switch (character) {
      case 0x04: report(character); break; // use filled characters & text cursor
      case 0x05: report(character); // use transparent characters & graphics cursor
      case 0x07: report(character); // play bell
      case 0x08: move_cursor_left(); break;
      case 0x09: move_cursor_right(); break;
      case 0x0A: move_cursor_down(); break;
      case 0x0B: move_cursor_up(); break;
      case 0x0C: clear_screen(); break;
      case 0x0D: move_cursor_boln(); break;
      case 0x0E: report(character); // paged mode ON
      case 0x0F: report(character); // paged mode OFF
      case 0x10: report(character); // clear graphics screen
      case 0x11: report(character); // set graphics color
      case 0x12: report(character); // set graphics column
      case 0x13: report(character); // define logical color (palette)
      case 0x16: report(character); // set vdu mode
      case 0x17: report(character); // handle vdu system command
      case 0x18: report(character); // define graphics viewport
      case 0x19: report(character); // vdu plot
      case 0x1A: report(character); // reset text and graphic viewports
      case 0x1C: report(character); // define text viewport
      case 0x1D: report(character); // set vdu origin
      case 0x1E: move_cursor_home(); break;
      case 0x1F: move_cursor_tab(); break;
      case 0x7F: do_backspace(); break;
      default: report(character); break;
    }
  }
}

void DiTerminal::process_string(const int8_t* string) {
  while (uint8_t character = *string++) {
    process_character(character);
  }
}

void DiTerminal::write_character(int8_t character) {
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

  // Set the tile bitmap index using the character code.
  set_tile(m_current_column, m_current_row, character);

  // Advance the current position
  if (++m_current_column >= m_columns) {
    m_current_column = 0;
    m_current_row++;
  }
}

void DiTerminal::write_character(int32_t column, int32_t row, int8_t character) {
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
  int32_t size_to_copy = columns * sizeof(uint32_t*);
  if (delta_vert > 0) {
    // moving rows down; copy bottom-up
    row += rows - 1;
    while (rows-- > 0) {
      uint32_t** dst = &m_tiles[row * m_words_per_row + column];
      uint32_t** src = &m_tiles[(row - delta_vert) * m_words_per_row + column + delta_horiz];
      memcpy(dst, src, size_to_copy);
      row--;
    }
  } else {
    // moving rows up; copy top-down
    while (rows-- > 0) {
      uint32_t** src = &m_tiles[row * m_words_per_row + column];
      uint32_t** dst = &m_tiles[(row + delta_vert) * m_words_per_row + column + delta_horiz];
      memcpy(dst, src, size_to_copy);
      row++;
    }
  }
}

void DiTerminal::clear_screen() {
  erase_text(0, 0, m_columns, m_rows);
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
  if (m_current_row < m_rows - 1) {
    m_current_row++;
  }
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

void DiTerminal::move_cursor_tab() {
  int32_t tab = (m_current_column & 0x3) + 4;
  if (tab < m_columns) {
    m_current_column = tab;
  }
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

void DiTerminal::report(uint8_t character) {
  process_character('[');
  process_character(to_hex(character >> 4));
  process_character(to_hex(character & 0xF));
  process_character(']');
}

uint8_t DiTerminal::to_hex(uint8_t value) {
  if (value < 10) {
    return value + 0x30; // '0'
  } else {
    return value - 10 + 0x41; // 'A'
  }
}