// di_terminal.h - Function declarations for supporting a character terminal display
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

#pragma once
#include "di_tile_map.h"

#define INCOMING_DATA_BUFFER_SIZE  2048

class DiTerminal: public DiTileMap {
  public:

  // Construct a terminal. Because the terminal is a tile map, this will allocate
  // data for the tile map. The terminal always shows characters that are 8x8
  // pixels, based on the built-in Agon font.
  //
  // The given x coordinate must be a multiple of 4, to align the terminal on
  // a 4-byte boundary, which saves memory and processing time.
  //
  // The given colors are used to iniialize the character bitmaps, but individual
  // characters may be defined (modified or replaced) by the application later.
  //
  // Example:
  // DiManager manager;
  // DiTerminal* terminal = manager.create_terminal(0, 0, 128, 100, 75, 0x3C, 0x10);
  //
  DiTerminal(uint32_t x, uint32_t y, uint32_t codes, uint32_t columns, uint32_t rows,
            uint8_t fg_color, uint8_t bg_color, const uint8_t* font);

  // Destroy a terminal, including its allocated data.
  virtual ~DiTerminal();

  // Store an incoming character for use later.
  void store_character(uint8_t character);

  // Store an incoming character string for use later.
  // The string is null-terminated.
  void store_string(const uint8_t* string);

  // Process all stored characters.
  void process_stored_characters();

  // Set the current character position. The position given may be within the terminal
  // display, or may be outside of it. If it is within the display, then the next
  // character written by write_character(ch) will appear at the given position. If the
  // position is outside of the display, then writing the next character will cause the
  // terminal display to scroll far enough to bring the current character position into
  // view, and the current position will be updated accordingly.
  void set_position(int32_t column, int32_t row);

  // Process an incoming character, which could be printable data or part of some
  // VDU command. If the character is printable, it will be written to the terminal
  // display. If the character is non-printable, or part of a VDU command, it will
  // be treated accordingly.
  void process_character(int8_t character);

  // Process an incoming string, which could be printable data and/or part of some
  // VDU command(s). This function calls process_character(), for each character
  // in the given string. The string is null-terminated.
  void process_string(const int8_t* string);

  // Write a character at the current character position. This may cause scrolling
  // BEFORE writing the character (not after), if the current character position is
  // off the visible terminal area. This function will advance the current character
  // position. The character is treated as a tile bitmap index, and is not interpreted
  // as a terminal command of any kind.
  void write_character(int8_t character);

  // Set the bitmap index to use to draw a character at a specific row and column.
  // This function does not cause scrolling, nor does it change the current
  // character position. The character is treated as a tile bitmap index, and is not
  // interpreted as a terminal command of any kind.
  void write_character(int32_t column, int32_t row, int8_t character);

  // Read the character code at the current character position. If the current position
  // is outside of the terminal display, this function returns zero.
  uint8_t read_character();

  // Read the character code at the given character position.
  uint8_t read_character(int32_t column, int32_t row);

  // Erase an area of text within the terminal display.
  void erase_text(int32_t column, int32_t row, int32_t columns, int32_t rows);

  // Move an area of text within the terminal display. This may be used to scroll
  // text at the character level (not at the pixel level).
  void move_text(int32_t column, int32_t row, int32_t columns, int32_t rows,
                  int32_t delta_horiz, int32_t delta_vert);

  void clear_screen();
  void move_cursor_left();
  void move_cursor_right();
  void move_cursor_down();
  void move_cursor_up();
  void move_cursor_home();
  void move_cursor_tab();
  void move_cursor_boln();
  void do_backspace();
  void report(uint8_t character);
  static uint8_t to_hex(uint8_t value);

  protected:
  int32_t   m_current_column;
  int32_t   m_current_row;
  uint32_t  m_next_buffer_write;
  uint32_t  m_next_buffer_read;
  uint32_t  m_num_buffer_chars;
  uint8_t   m_incoming_data[INCOMING_DATA_BUFFER_SIZE];
};

