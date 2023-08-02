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
#include "../agon.h"

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
  m_num_command_chars = 0;

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
    bool rc = process_character(m_incoming_data[m_next_buffer_read++]);
    if (m_next_buffer_read >= INCOMING_DATA_BUFFER_SIZE) {
      m_next_buffer_read = 0;
    }
    m_num_buffer_chars--;
    if (!rc) {
      break; // need to wait for more data
    }
  }
}

void DiTerminal::set_position(int32_t column, int32_t row) {
  m_current_column = column;
  m_current_row = row;
}

bool DiTerminal::process_character(int8_t character) {
  if (m_num_command_chars) {
    return handle_udg_sys_cmd(character);
  } else if (character >= 0x20 && character != 0x7F) {
    // printable character
    write_character(character);
  } else {
    switch (character) {
      case 0x04: report(character); break; // use filled characters & text cursor
      case 0x05: report(character); break; // use transparent characters & graphics cursor
      case 0x07: report(character); break; // play bell
      case 0x08: move_cursor_left(); break;
      case 0x09: move_cursor_right(); break;
      case 0x0A: move_cursor_down(); break;
      case 0x0B: move_cursor_up(); break;
      case 0x0C: clear_screen(); break;
      case 0x0D: move_cursor_boln(); break;
      case 0x0E: report(character); break; // paged mode ON
      case 0x0F: report(character); break; // paged mode OFF
      case 0x10: report(character); break; // clear graphics screen
      case 0x11: report(character); break; // set color
      case 0x12: report(character); break; // set graphics mode, color
      case 0x13: report(character); break; // define logical color (palette)
      case 0x16: report(character); break; // set vdu mode
      case 0x17: return handle_udg_sys_cmd(character); // handle UDG/system command
      case 0x18: report(character); break; // define graphics viewport
      case 0x19: report(character); break; // vdu plot
      case 0x1A: report(character); break; // reset text and graphic viewports
      case 0x1C: report(character); break; // define text viewport
      case 0x1D: report(character); break; // set graphics origin
      case 0x1E: move_cursor_home(); break;
      case 0x1F: move_cursor_tab(); break;
      case 0x7F: do_backspace(); break;
      default: report(character); break;
    }
  }
  return true;
}

void DiTerminal::process_string(const int8_t* string) {
  while (uint8_t character = *string++) {
    if (!process_character(character)) {
      break;
    }
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

uint8_t DiTerminal::peek_into_buffer() {
  return m_incoming_data[m_next_buffer_read];
}

uint8_t DiTerminal::read_from_buffer() {
  uint8_t character = m_incoming_data[m_next_buffer_read++];
  if (m_next_buffer_read >= INCOMING_DATA_BUFFER_SIZE) {
    m_next_buffer_read = 0;
  }
  m_num_buffer_chars--;
  return character;
}

void DiTerminal::skip_from_buffer() {
  if (++m_next_buffer_read >= INCOMING_DATA_BUFFER_SIZE) {
    m_next_buffer_read = 0;
  }
  m_num_buffer_chars--;
}

/*
VDU 23, 0, &80, b: General poll
VDU 23, 0, &81, n: Set the keyboard locale (0=UK, 1=US, etc)
VDU 23, 0, &82: Request cursor position
VDU 23, 0, &83, x; y;: Get ASCII code of character at character position x, y
VDU 23, 0, &84, x; y;: Get colour of pixel at pixel position x, y
VDU 23, 0, &85, channel, waveform, volume, freq; duration;: Send a note to the VDP audio driver
VDU 23, 0, &86: Fetch the screen dimensions
VDU 23, 0, &87: RTC control (Requires MOS 1.03 or above)
VDU 23, 0, &88, delay; rate; led: Keyboard Control (Requires MOS 1.03 or above)
VDU 23, 0, &C0, n: Turn logical screen scaling on and off, where 1=on and 0=off (Requires MOS 1.03 or above)
VDU 23, 0, &FF: Switch to terminal mode for CP/M (This will disable keyboard entry in BBC BASIC/MOS)
*/
bool DiTerminal::handle_udg_sys_cmd(uint8_t character) {
  m_incoming_command[m_num_command_chars++] = character;
  if (m_num_command_chars >= 3) {
    switch (m_incoming_command[2]) {

      case VDP_GP: /*0x80*/ {
        if (m_num_command_chars == 4) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      case VDP_KEYCODE: /*0x81*/ {
        if (m_num_command_chars == 4) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      case VDP_CURSOR: /*0x82*/ {
        if (m_num_command_chars == 3) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      case VDP_SCRCHAR: /*0x83*/ {
        if (m_num_command_chars == 7) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      case VDP_SCRPIXEL: /*0x84*/ {
        if (m_num_command_chars == 7) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      case VDP_AUDIO: /*0x85*/ {
        if (m_num_command_chars == 10) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      case VDP_MODE: /*0x86*/ {
        if (m_num_command_chars == 3) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      case VDP_RTC: /*0x87*/ {
        if (m_num_command_chars == 3) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      case VDP_KEYSTATE: /*0x88*/ {
        if (m_num_command_chars == 8) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      case VDP_LOGICALCOORDS: /*0xC0*/ {
        if (m_num_command_chars == 4) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      case VDP_TERMINALMODE: /*0xFF*/ {
        if (m_num_command_chars == 3) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

    }
  }
  return false;
}