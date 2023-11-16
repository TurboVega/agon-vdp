// di_manager.h - Function declarations for managing drawing-instruction primitives
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

#include <vector>
#include <map>
#include "rom/lldesc.h"
#include "di_video_buffer.h"
#include "di_terminal.h"
#include "di_tile_map.h"
#include "di_render.h"
#include "di_solid_rectangle.h"
#include "di_commands.h"

typedef void (*DiVoidCallback)();

#define INCOMING_DATA_BUFFER_SIZE  2048
#define INCOMING_COMMAND_SIZE      24

class DiManager {
    public:
    // Construct a drawing-instruction manager, which handles multiple drawing primitives.
    DiManager();

    // Destroy the manager and its child primitives.
    ~DiManager();

    // Create the root primitive.
    void create_root();

    // Create various types of drawing primitives.
    DiPrimitive* create_point(uint16_t id, uint16_t parent, uint16_t flags,
                            int32_t x, int32_t y, uint8_t color);

    DiPrimitive* create_line(uint16_t id, uint16_t parent, uint16_t flags,
                            int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                            uint8_t color);

    DiPrimitive* create_rectangle_outline(OtfCmd_40_Create_primitive_Rectangle_Outline* cmd);

    DiSolidRectangle* create_solid_rectangle(OtfCmd_41_Create_primitive_Solid_Rectangle* cmd);

    DiPrimitive* create_ellipse(uint16_t id, uint16_t parent, uint16_t flags,
                            int32_t x, int32_t y, uint32_t width, uint32_t height,
                            uint8_t color);

    DiPrimitive* create_solid_ellipse(uint16_t id, uint16_t parent, uint16_t flags,
                            int32_t x, int32_t y, uint32_t width, uint32_t height,
                            uint8_t color);

    DiPrimitive* create_triangle_outline(OtfCmd_30_Create_primitive_Triangle_Outline* cmd);

    DiPrimitive* create_solid_triangle(OtfCmd_31_Create_primitive_Solid_Triangle* cmd);

    DiPrimitive* create_triangle_list_outline(OtfCmd_32_Create_primitive_Triangle_List_Outline* cmd);
    
    DiPrimitive* create_solid_triangle_list(OtfCmd_33_Create_primitive_Solid_Triangle_List* cmd);

    DiPrimitive* create_triangle_fan_outline(OtfCmd_34_Create_primitive_Triangle_Fan_Outline* cmd);

    DiPrimitive* create_solid_triangle_fan(OtfCmd_35_Create_primitive_Solid_Triangle_Fan* cmd);

    DiPrimitive* create_triangle_strip_outline(OtfCmd_36_Create_primitive_Triangle_Strip_Outline* cmd);

    DiPrimitive* create_solid_triangle_strip(OtfCmd_37_Create_primitive_Solid_Triangle_Strip* cmd);

    DiPrimitive* create_quad_outline(OtfCmd_60_Create_primitive_Quad_Outline* cmd);

    DiPrimitive* create_solid_quad(OtfCmd_61_Create_primitive_Solid_Quad* cmd);

    DiPrimitive* create_quad_list_outline(OtfCmd_62_Create_primitive_Quad_List_Outline* cmd);
    
    DiPrimitive* create_solid_quad_list(OtfCmd_63_Create_primitive_Solid_Quad_List* cmd);

    DiPrimitive* create_quad_strip_outline(OtfCmd_64_Create_primitive_Quad_Strip_Outline* cmd);

    DiPrimitive* create_solid_quad_strip(OtfCmd_65_Create_primitive_Solid_Quad_Strip* cmd);

    DiTileMap* create_tile_map(uint16_t id, uint16_t parent, uint16_t flags,
                            int32_t screen_width, int32_t screen_height,
                            uint32_t columns, uint32_t rows, uint32_t width, uint32_t height);

    DiTileArray* create_tile_array(uint16_t id, uint16_t parent, uint16_t flags,
                            int32_t screen_width, int32_t screen_height,
                            uint32_t columns, uint32_t rows, uint32_t width, uint32_t height);

    DiTerminal* create_terminal(uint16_t id, uint16_t parent, uint16_t flags,
                            uint32_t x, uint32_t y, uint32_t columns, uint32_t rows, const uint8_t* font);

    DiBitmap* create_solid_bitmap(uint16_t id, uint16_t parent, uint16_t flags,
                            uint32_t width, uint32_t height);

    DiBitmap* create_masked_bitmap(uint16_t id, uint16_t parent, uint16_t flags,
                            uint32_t width, uint32_t height, uint8_t color);

    DiBitmap* create_transparent_bitmap(uint16_t id, uint16_t parent, uint16_t flags,
                            uint32_t width, uint32_t height, uint8_t color);

    DiBitmap* create_reference_solid_bitmap(uint16_t id, uint16_t parent, uint16_t flags, uint16_t bmid);

    DiBitmap* create_reference_masked_bitmap(uint16_t id, uint16_t parent, uint16_t flags, uint16_t bmid);

    DiBitmap* create_reference_transparent_bitmap(uint16_t id, uint16_t parent, uint16_t flags, uint16_t bmid);

    DiTileBitmap* create_solid_bitmap_for_tile_array(uint16_t id, uint16_t bm_id);

    DiTileBitmap* create_masked_bitmap_for_tile_array(uint16_t id, uint16_t bm_id, uint8_t color);

    DiTileBitmap* create_transparent_bitmap_for_tile_array(uint16_t id, uint16_t bm_id, uint8_t color);

    DiTileBitmap* create_solid_bitmap_for_tile_map(uint16_t id, uint16_t bm_id);

    DiTileBitmap* create_masked_bitmap_for_tile_map(uint16_t id, uint16_t bm_id, uint8_t color);

    DiTileBitmap* create_transparent_bitmap_for_tile_map(uint16_t id, uint16_t bm_id, uint8_t color);

    DiRender* create_solid_render(uint16_t id, uint16_t parent, uint16_t flags,
                            uint32_t width, uint32_t height);

    DiRender* create_masked_render(uint16_t id, uint16_t parent, uint16_t flags,
                            uint32_t width, uint32_t height);

    DiRender* create_transparent_render(uint16_t id, uint16_t parent, uint16_t flags,
                            uint32_t width, uint32_t height, uint8_t color);

    DiPrimitive* create_primitive_group(OtfCmd_140_Create_primitive_Group* cmd);

    // Set the flags for an existing primitive.
    void set_primitive_flags(uint16_t id, uint16_t flags);

    // Move an existing primitive to an absolute position.
    void move_primitive_absolute(uint16_t id, int32_t x, int32_t y);

    // Move an existing primitive to a relative position.
    void move_primitive_relative(uint16_t id, int32_t x, int32_t y);

    // Delete an existing primitive.
    void delete_primitive(uint16_t id);

    // Generate code for an existing primitive.
    void generate_code_for_primitive(uint16_t id);

    // Move an existing bitmap to an absolute position and slice it.
    void slice_solid_bitmap_absolute(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height);
    void slice_masked_bitmap_absolute(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height);
    void slice_transparent_bitmap_absolute(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height);

    // Move an existing bitmap to a relative position and slice it.
    void slice_solid_bitmap_relative(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height);
    void slice_masked_bitmap_relative(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height);
    void slice_transparent_bitmap_relative(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height);

    // Set a pixel within an existing bitmap.
    void set_solid_bitmap_pixel(uint16_t id, int32_t x, int32_t y, uint8_t color, int16_t nth);
    void set_masked_bitmap_pixel(uint16_t id, int32_t x, int32_t y, uint8_t color, int16_t nth);
    void set_transparent_bitmap_pixel(uint16_t id, int32_t x, int32_t y, uint8_t color, int16_t nth);
    void set_solid_bitmap_pixel_for_tile_array(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth);
    void set_masked_bitmap_pixel_for_tile_array(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth);
    void set_transparent_bitmap_pixel_for_tile_array(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth);
    void set_solid_bitmap_pixel_for_tile_map(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth);
    void set_masked_bitmap_pixel_for_tile_map(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth);
    void set_transparent_bitmap_pixel_for_tile_map(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth);

    // Set bitmap ID for tile in tile array.
    void set_tile_array_bitmap_id(uint16_t id, uint16_t col, uint16_t row, uint16_t bm_id);

    // Set bitmap ID for tile in tile map.
    void set_tile_map_bitmap_id(uint16_t id, uint16_t col, uint16_t row, uint16_t bm_id);

    // Setup a callback for when the visible frame pixels have been sent to DMA,
    // and the vertical blanking time begins.
    void set_on_vertical_blank_cb(DiVoidCallback callback_fcn);

    // Setup and run the main loop to do continuous drawing.
    // For the demo, the loop never ends.
    void IRAM_ATTR run();

    // Store an incoming character for use later.
    void store_character(uint8_t character);

    // Store an incoming character string for use later.
    // The string is null-terminated.
    void store_string(const uint8_t* string);

    // Validate a primitive ID.
    inline bool validate_id(int16_t id) { return (id >= 0) && (id < MAX_NUM_PRIMITIVES); }
  
    // Get a safe primitive pointer.
    inline DiPrimitive* get_safe_primitive(int16_t id) { return validate_id(id) ? m_primitives[id] : NULL; }

    protected:
    // Structures used to support DMA for video.
    volatile lldesc_t *         m_dma_descriptor; // [DMA_TOTAL_DESCR]
    volatile DiVideoBuffer *    m_video_buffer; // [NUM_ACTIVE_BUFFERS]
    volatile DiVideoScanLine *  m_front_porch;
    volatile DiVideoBuffer *    m_vertical_sync;
    volatile DiVideoScanLine *  m_back_porch;
    DiVoidCallback              m_on_vertical_blank_cb;
    uint32_t                    m_next_buffer_write;
    uint32_t                    m_next_buffer_read;
    uint32_t                    m_num_buffer_chars;
    uint32_t                    m_command_data_index;
    DiTerminal*                 m_terminal;
    DiSolidRectangle*           m_cursor;
    uint8_t                     m_flash_count;
    uint8_t                     m_incoming_data[INCOMING_DATA_BUFFER_SIZE];
    std::vector<uint8_t>        m_incoming_command;
    DiPrimitive *               m_primitives[MAX_NUM_PRIMITIVES]; // Indexes of array are primitive IDs
    std::vector<DiPrimitive*>   m_groups[ACT_LINES]; // Vertical scan groups (for optimizing paint calls)

    // Setup the DMA stuff.
    void initialize();

    // Run the main loop.
    void IRAM_ATTR loop();

    // Clear the primitive data, etc.
    void clear();

    // Add a primitive to the manager.
    void add_primitive(DiPrimitive* prim, DiPrimitive* parent);

    // Delete a primitive from the manager.
    void remove_primitive(DiPrimitive* prim);

    // Recompute the geometry and paint list membership for a primitive.
    void recompute_primitive(DiPrimitive* prim, uint16_t old_flags,
                             int32_t old_min_group, int32_t old_max_group);
    // Finish creating a primitive.
    DiPrimitive* finish_create(uint16_t id, uint16_t flags, DiPrimitive* prim, DiPrimitive* parent_prim);

    // Draw all primitives that belong to the active scan line group.
    void IRAM_ATTR draw_primitives(volatile uint32_t* p_scan_line, uint32_t line_index);

    // Setup a single DMA descriptor.
    void init_dma_descriptor(volatile DiVideoScanLine* vbuf, uint32_t descr_index);

    // Setup a pair of DMA descriptors.
    void init_dma_descriptor(volatile DiVideoBuffer* vbuf, uint32_t descr_index);

  // Process all stored characters.
  void process_stored_characters();

  // Process an incoming character, which could be printable data or part of some
  // VDU command. If the character is printable, it will be written to the terminal
  // display. If the character is non-printable, or part of a VDU command, it will
  // be treated accordingly. This function returns true if the character was fully
  // processed, and false otherwise.
  bool process_character(uint8_t character);

  // Process an incoming string, which could be printable data and/or part of some
  // VDU command(s). This function calls process_character(), for each character
  // in the given string. The string is null-terminated.
  void process_string(const uint8_t* string);

  uint8_t get_param_8(uint32_t index);
  int16_t get_param_16(uint32_t index);
  bool handle_udg_sys_cmd(uint8_t character);
  bool handle_otf_cmd();
  bool ignore_cmd(uint8_t character, uint8_t len);
  bool define_graphics_viewport(uint8_t character);
  bool define_text_viewport(uint8_t character);
  bool move_cursor_tab(uint8_t character);
  void clear_screen();
  void move_cursor_left();
  void move_cursor_right();
  void move_cursor_down();
  void move_cursor_up();
  void move_cursor_home();
  void move_cursor_boln();
  void do_backspace();
  uint8_t read_character(int16_t x, int16_t y);
  void write_character(uint8_t character);
  void report(uint8_t character);
  static uint8_t to_hex(uint8_t value);
  uint8_t peek_into_buffer();
  uint8_t read_from_buffer();
  void skip_from_buffer();
  void send_cursor_position();
  void send_screen_char(int16_t x, int16_t y);
  void send_screen_pixel(int16_t x, int16_t y);
  void send_mode_information();
  void send_general_poll(uint8_t b);
};