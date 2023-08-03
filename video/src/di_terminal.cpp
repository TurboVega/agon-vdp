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

// These things are defined in video.ino, already.
typedef uint8_t byte;
void send_packet(byte code, byte len, byte data[]);
void sendTime();
void sendKeyboardState();
void sendPlayNote(int channel, int success);
void vdu_sys_video_kblayout(byte region);
extern bool initialised;
extern bool logicalCoords;
extern bool terminalMode;
extern int videoMode;

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

  logicalCoords = false; // this mode always uses regular coordinates
  terminalMode = true; // this mode is terminal mode

  // Copy built-in font pixel data to the bitmaps for this terminal.
  for (int b = 0; b < codes; b++) {
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

/*
From Agon Wiki: https://github.com/breakintoprogram/agon-docs/wiki/VDP
VDU 8: Cursor left
VDU 9: Cursor right
VDU 10: Cursor down
VDU 11: Cursor up
VDU 12: CLS
VDU 13: Carriage return
VDU 14: Page mode ON (VDP 1.03 or greater)
VDU 15: Page mode OFF (VDP 1.03 or greater)
VDU 16: CLG
VDU 17 colour: COLOUR colour
VDU 18, mode, colour: GCOL mode, colour
VDU 19, l, p, r, g, b: COLOUR l, p / COLOUR l, r, g, b
VDU 22, n: Mode n
VDU 23, n: UDG / System Commands
VDU 24, left; bottom; right; top;: Set graphics viewport (VDP 1.04 or greater)
VDU 25, mode, x; y;: PLOT mode, x, y
VDU 26: Reset graphics and text viewports (VDP 1.04 or greater)
VDU 28, left, bottom, right, top: Set text viewport (VDP 1.04 or greater)
VDU 29, x; y;: Graphics origin
VDU 30: Home cursor
VDU 31, x, y: TAB(x, y)
VDU 127: Backspace
*/
bool DiTerminal::process_character(int8_t character) {
  if (m_num_command_chars) {
    switch (m_incoming_command[0]) {
      case 0x17: return handle_udg_sys_cmd(character); // handle UDG/system command
      case 0x1F: return move_cursor_tab(character);
    }
    return false;
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
      case 0x18: return define_graphics_viewport(character);
      case 0x19: report(character); break; // vdu plot
      case 0x1A: clear_screen(); break; // reset text and graphic viewports
      case 0x1C: return define_text_viewport(character);
      case 0x1D: report(character); break; // set graphics origin
      case 0x1E: move_cursor_home(); break;
      case 0x1F: return move_cursor_tab(character);
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

bool DiTerminal::move_cursor_tab(uint8_t character) {
  m_incoming_command[m_num_command_chars++] = character;
  if (m_num_command_chars >= 3) {
      int8_t x = get_param_8(1);
      int8_t y = get_param_8(2);
      set_position(x, y);
      m_num_command_chars = 0;
      return true;
  }
  return false;
}

bool DiTerminal::define_graphics_viewport(uint8_t character) {
  m_incoming_command[m_num_command_chars++] = character;
  if (m_num_command_chars >= 9) {
      int16_t left = get_param_16(1);
      int16_t bottom = get_param_16(3);
      int16_t right = get_param_16(5);
      int16_t top = get_param_16(7);
      m_num_command_chars = 0;
      return true;
  }
  return false;
}

bool DiTerminal::define_text_viewport(uint8_t character) {
  m_incoming_command[m_num_command_chars++] = character;
  if (m_num_command_chars >= 5) {
      int8_t left = get_param_8(1);
      int8_t bottom = get_param_8(2);
      int8_t right = get_param_8(3);
      int8_t top = get_param_8(4);
      m_num_command_chars = 0;
      return true;
  }
  return false;
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
  write_character('[');
  write_character(to_hex(character >> 4));
  write_character(to_hex(character & 0xF));
  write_character(']');
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
From Agon Wiki: https://github.com/breakintoprogram/agon-docs/wiki/VDP
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

From Julian Regel's tile map commands: https://github.com/julianregel/agonnotes
VDU 23, 0, &C2, 0:	Initialise/Reset Tile Layer
VDU 23, 0, &C2, 1:	Set Layer Properties
VDU 23, 0, &C2, 2:	Set Tile Properties
VDU 23, 0, &C2, 3:	Draw Layer
VDU 23, 0, &C4, 0:	Set Border Colour
VDU 23, 0, &C4, 1:	Draw Border
*/
bool DiTerminal::handle_udg_sys_cmd(uint8_t character) {
  m_incoming_command[m_num_command_chars++] = character;
  if (m_num_command_chars >= 2 && get_param_8[1] == 30) {
    return handle_otf_cmd();
  }
  if (m_num_command_chars >= 3) {
    switch (m_incoming_command[2]) {

      // VDU 23, 0, &80, b: General poll
      case VDP_GP: /*0x80*/ {
        if (m_num_command_chars == 4) {
          int8_t echo = get_param_8(3);
          send_general_poll(echo);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 0, &81, n: Set the keyboard locale (0=UK, 1=US, etc)
      case VDP_KEYCODE: /*0x81*/ {
        if (m_num_command_chars == 4) {
          int8_t region = get_param_8(3);
          vdu_sys_video_kblayout(region);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 0, &82: Request cursor position
      case VDP_CURSOR: /*0x82*/ {
        if (m_num_command_chars == 3) {
          send_cursor_position();
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 0, &83, x; y;: Get ASCII code of character at character position x, y
      case VDP_SCRCHAR: /*0x83*/ {
        if (m_num_command_chars == 7) {
          int32_t x = get_param_16(3);
          int32_t y = get_param_16(5);
          send_screen_char(x, y);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 0, &84, x; y;: Get colour of pixel at pixel position x, y
      case VDP_SCRPIXEL: /*0x84*/ {
        if (m_num_command_chars == 7) {
          int32_t x = get_param_16(3);
          int32_t y = get_param_16(5);
          send_screen_pixel(x, y);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 0, &85, channel, waveform, volume, freq; duration;: Send a note to the VDP audio driver
      case VDP_AUDIO: /*0x85*/ {
        if (m_num_command_chars == 10) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 0, &86: Fetch the screen dimensions
      case VDP_MODE: /*0x86*/ {
        if (m_num_command_chars == 3) {
          send_mode_information();
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 0, &87: RTC control (Requires MOS 1.03 or above)
      case VDP_RTC: /*0x87*/ {
        if (m_num_command_chars == 3) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 0, &88, delay; rate; led: Keyboard Control (Requires MOS 1.03 or above)
      case VDP_KEYSTATE: /*0x88*/ {
        if (m_num_command_chars == 8) {
          sendKeyboardState();
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 0, &C0, n: Turn logical screen scaling on and off, where 1=on and 0=off (Requires MOS 1.03 or above)
      case VDP_LOGICALCOORDS: /*0xC0*/ {
        if (m_num_command_chars == 4) {
          // This command is ignored; this mode always uses regular coordinates.
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 0, &FF: Switch to terminal mode for CP/M (This will disable keyboard entry in BBC BASIC/MOS)
      case VDP_TERMINALMODE: /*0xFF*/ {
        if (m_num_command_chars == 3) {
          // This command is ignored; this mode is terminal mode.
          m_num_command_chars = 0;
          return true;
        }
      } break;

    }
  }
  return false;
}

/*
800x600x64 On-the-Fly Command Set:
VDU 23, 30, 0, p; e:                             Enable/disable primitive
VDU 23, 30, 1, p; x; y;:                         Move primitive: absolute
VDU 23, 30, 2, p; x; y;:                         Move primitive: relative
VDU 23, 30, 3, p;:                               Delete primitive
VDU 23, 30, 4, p; x; y; c:                       Create primitive: Pixel
VDU 23, 30, 5, p; x1; y1; x2; y2; c:             Create primitive: Line
VDU 23, 30, 6, p; x1; y1; x2; y2; x3; y3; c:     Create primitive: Triangle Outline
VDU 23, 30, 7, p; x1; y1; x2; y2; x3; y3; c:     Create primitive: Solid Triangle
VDU 23, 30, 8, p; x; y; w; h; c:                 Create primitive: Rectangle Outline
VDU 23, 30, 9, p; x; y; w; h; c:                 Create primitive: Solid Rectangle
VDU 23, 30, 10, p; x; y; w; h; c:                Create primitive: Ellipse Outline
VDU 23, 30, 11, p; x; y; w; h; c:                Create primitive: Solid Ellipse
VDU 23, 30, 12, p; bitmaps, cols; rows; w; h; hs: Create primitive: Tile Map
VDU 23, 30, 13, p; w; h; hs, vs:                 Create primitive: Solid Bitmap
VDU 23, 30, 14, p; w; h; hs, vs:                 Create primitive: Masked Bitmap
VDU 23, 30, 15, p; w; h; hs, vs, c:              Create primitive: Transparent Bitmap
VDU 23, 30, 16, p; x; y;:                        Create primitive: Group
VDU 23, 30, 17, p; x; y; s; h;:                  Move & slice bitmap: absolute
VDU 23, 30, 18, p; x; y; s; h;:                  Move & slice bitmap: relative
VDU 23, 30, 19, p; x; y; c:                      Set bitmap pixel
VDU 23, 30, 20, p; x; y; n; c0, c1, c2, ...:     Set bitmap pixels
VDU 23, 30, 21, p; g;:                           Add primitive to group
VDU 23, 30, 22, p; g;:                           Remove primitive from group
VDU 23, 30, 23, p; col, row, bi:                 Set bitmap index for tile in tile map
VDU 23, 30, 24, p; bi, x; y; c:                  Set bitmap pixel in tile map
VDU 23, 30, 25, p; bi, x; y; n; c0, c1, c2, ...: Set bitmap pixels in tile map
*/
bool DiTerminal::handle_otf_cmd() {
  if (m_num_command_chars >= 5) {
    int16_t p = get_param_16(3); // get primitive index number
    switch (m_incoming_command[2]) {

      // VDU 23, 30, 0, p; e: Enable/disable primitive
      case 0: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 1, p; x; y;: Move primitive: absolute
      case 1: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 2, p; x; y;: Move primitive: relative
      case 2: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 3, p;: Delete primitive
      case 3: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 4, p; x; y; c: Create primitive: Pixel
      case 4: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 5, p; x1; y1; x2; y2; c: Create primitive: Line
      case 5: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 6, p; x1; y1; x2; y2; x3; y3; c: Create primitive: Triangle Outline
      case 6: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 7, p; x1; y1; x2; y2; x3; y3; c: Create primitive: Solid Triangle
      case 7: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 8, p; x; y; w; h; c: Create primitive: Rectangle Outline
      case 8: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 9, p; x; y; w; h; c: Create primitive: Solid Rectangle
      case 9: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 10, p; x; y; w; h; c: Create primitive: Ellipse Outline
      case 10: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 11, p; x; y; w; h; c: Create primitive: Solid Ellipse
      case 11: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 12, p; bitmaps, cols; rows; w; h; hs: Create primitive: Tile Map
      case 12: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 13, p; w; h; hs, vs: Create primitive: Solid Bitmap
      case 13: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 14, p; w; h; hs, vs: Create primitive: Masked Bitmap
      case 14: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 15, p; w; h; hs, vs, c: Create primitive: Transparent Bitmap
      case 15: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 16, p; x; y;: Create primitive: Group
      case 16: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 17, p; x; y; s; h;: Move & slice bitmap: absolute
      case 17: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 18, p; x; y; s; h;: Move & slice bitmap: relative
      case 18: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 19, p; x; y; c: Set bitmap pixel
      case 19: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 20, p; x; y; n; c0, c1, c2, ...: Set bitmap pixels
      case 20: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 21, p; g;: Add primitive to group
      case 21: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 22, p; g;: Remove primitive from group
      case 22: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 23, p; col, row, bi: Set bitmap index for tile in tile map
      case 23: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 24, p; bi, x; y; c: Set bitmap pixel in tile map
      case 24: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 25, p; bi, x; y; n; c0, c1, c2, ...: Set bitmap pixels in tile map
      case 25: {
        if (m_num_command_chars == ) {
          m_num_command_chars = 0;
          return true;
        }
      } break;
    }
    return false;
}

int8_t DiTerminal::get_param_8(uint32_t index) {
  return m_incoming_command[index];
}

int16_t DiTerminal::get_param_16(uint32_t index) {
  return (((int16_t)m_incoming_command[index+1]) << 8) | m_incoming_command[index];
}

// Send the cursor position back to MOS
//
void DiTerminal::send_cursor_position() {
	byte packet[] = {
		(byte) m_current_column,
		(byte) m_current_row & 0xFF,
	};
	send_packet(PACKET_CURSOR, sizeof packet, packet);	
}

// Send a character back to MOS
//
void DiTerminal::send_screen_char(int32_t x, int32_t y) {
	uint8_t c = read_character(x, y);
	byte packet[] = {
		c,
	};
	send_packet(PACKET_SCRCHAR, sizeof packet, packet);
}

// Send a pixel value back to MOS
//
void DiTerminal::send_screen_pixel(int32_t x, int32_t y) {
	byte packet[] = {
		0,	// R
		0,  // G
		0,  // B
		0,	// There is no palette in this mode.
	};
	send_packet(PACKET_SCRPIXEL, sizeof packet, packet);	
}

// Send MODE information (screen details)
//
void DiTerminal::send_mode_information() {
	byte packet[] = {
		ACT_PIXELS & 0xFF,	 				// Width in pixels (L)
		(ACT_PIXELS >> 8) & 0xFF,		// Width in pixels (H)
		ACT_LINES & 0xFF,						// Height in pixels (L)
		(ACT_LINES >> 8) & 0xFF,		// Height in pixels (H)
		(ACT_PIXELS / 8),					  // Width in characters (byte)
		(ACT_LINES / 8),					  // Height in characters (byte)
		64,						              // Colour depth
		videoMode & 0xFF            // The video mode number
	};
	send_packet(PACKET_MODE, sizeof packet, packet);
}

// Send a general poll
//
void DiTerminal::send_general_poll(uint8_t b) {
	byte packet[] = {
		b,
	};
	send_packet(PACKET_GP, sizeof packet, packet);
	initialised = true;	
}
