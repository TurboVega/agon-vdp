// di_manager.cpp - Function definitions for managing drawing-instruction primitives
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

#include "di_manager.h"

#include <stddef.h>
#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "driver/periph_ctrl.h"
#include "soc/rtc.h"
#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "fabgl_pieces.h"

#include "di_diag_left_line.h"
#include "di_diag_right_line.h"
#include "di_general_line.h"
#include "di_horiz_line.h"
#include "di_set_pixel.h"
#include "di_vert_line.h"
#include "di_ellipse.h"
#include "di_solid_ellipse.h"
#include "di_rectangle.h"
#include "di_solid_rectangle.h"
#include "di_tile_map.h"
#include "di_opaque_bitmap.h"
#include "di_masked_bitmap.h"
#include "di_transparent_bitmap.h"

#include "../agon.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "ESP32Time.h"
#include "HardwareSerial.h"

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

typedef enum {
  WritingActiveLines,
  ProcessingIncomingData,
  NearNewFrameStart
} LoopState;

// Default callback functions
void default_on_vertical_blank() {}
void default_on_lines_painted() {}

DiManager::DiManager() {
  m_next_buffer_write = 0;
  m_next_buffer_read = 0;
  m_num_buffer_chars = 0;
  m_num_command_chars = 0;
  m_terminal = NULL;
  m_on_vertical_blank_cb = &default_on_vertical_blank;
  m_on_lines_painted_cb = &default_on_lines_painted;
  memset(m_primitives, 0, sizeof(m_primitives));

  logicalCoords = false; // this mode always uses regular coordinates
  terminalMode = false; // this mode is not (yet) terminal mode

  // The root primitive covers the entire screen, and is not drawn.
  // The application should define what the base layer of the screen
  // is (e.g., solid rectangle, terminal, tile map, etc.).

  DiPrimitive* root = new DiPrimitive;
  root->init_root();
  m_primitives[ROOT_PRIMITIVE_ID] = root;
}

DiManager::~DiManager() {
    clear();
}

void DiManager::initialize() {
  size_t new_size = (size_t)(sizeof(lldesc_t) * DMA_TOTAL_DESCR);
  void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA);
  m_dma_descriptor = (volatile lldesc_t *)p;

  new_size = (size_t)(sizeof(DiVideoBuffer) * NUM_ACTIVE_BUFFERS);
  p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA);
  m_video_buffer = (volatile DiVideoBuffer *)p;

  new_size = (size_t)(sizeof(DiVideoScanLine));
  p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA);
  m_front_porch = (volatile DiVideoScanLine *)p;

  new_size = (size_t)(sizeof(DiVideoBuffer));
  p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA);
  m_vertical_sync = (volatile DiVideoBuffer *)p;

  new_size = (size_t)(sizeof(DiVideoScanLine));
  p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_8BIT|MALLOC_CAP_DMA);
  m_back_porch = (volatile DiVideoScanLine *)p;

  // DMA buffer chain: ACT
  uint32_t descr_index = 0;
  for (uint32_t i = 0; i < NUM_ACTIVE_BUFFERS; i++) {
    m_video_buffer[i].init_to_black();
  }
  for (uint32_t i = 0; i < ACT_BUFFERS_WRITTEN; i++) {
    init_dma_descriptor(&m_video_buffer[i & (NUM_ACTIVE_BUFFERS - 1)], descr_index++);
  }

  // DMA buffer chain: VFP
  m_front_porch->init_to_black();
  for (uint i = 0; i < VFP_LINES; i++) {
    init_dma_descriptor(m_front_porch, descr_index++);
  }

  // DMA buffer chain: VS
  m_vertical_sync->init_for_vsync();
  for (uint i = 0; i < VS_LINES/NUM_LINES_PER_BUFFER; i++) {
    init_dma_descriptor(m_vertical_sync, descr_index++);
  }
  
  // DMA buffer chain: VBP
  m_back_porch->init_to_black();
  for (uint i = 0; i < VBP_LINES; i++) {
    init_dma_descriptor(m_back_porch, descr_index++);
  }

  // GPIO configuration for color bits
  setupGPIO(GPIO_RED_0,   VGA_RED_BIT,   GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_RED_1,   VGA_RED_BIT + 1,   GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_GREEN_0, VGA_GREEN_BIT, GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_GREEN_1, VGA_GREEN_BIT + 1, GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_BLUE_0,  VGA_BLUE_BIT,  GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_BLUE_1,  VGA_BLUE_BIT + 1,  GPIO_MODE_OUTPUT);

  // GPIO configuration for VSync and HSync
  setupGPIO(GPIO_HSYNC, VGA_HSYNC_BIT, GPIO_MODE_OUTPUT);
  setupGPIO(GPIO_VSYNC, VGA_VSYNC_BIT, GPIO_MODE_OUTPUT);

  // Start the DMA

  // Power on device
  periph_module_enable(PERIPH_I2S1_MODULE);

  // Initialize I2S device
  I2S1.conf.tx_reset = 1;
  I2S1.conf.tx_reset = 0;

  // Reset DMA
  I2S1.lc_conf.out_rst = 1;
  I2S1.lc_conf.out_rst = 0;

  // Reset FIFO
  I2S1.conf.tx_fifo_reset = 1;
  I2S1.conf.tx_fifo_reset = 0;

  // LCD mode
  I2S1.conf2.val            = 0;
  I2S1.conf2.lcd_en         = 1;
  I2S1.conf2.lcd_tx_wrx2_en = 0; // NOT 1!
  I2S1.conf2.lcd_tx_sdx2_en = 0;

  I2S1.sample_rate_conf.val         = 0;
  I2S1.sample_rate_conf.tx_bits_mod = 8;

  setup_dma_clock(DMA_CLOCK_FREQ);

  I2S1.fifo_conf.val                  = 0;
  I2S1.fifo_conf.tx_fifo_mod_force_en = 1;
  I2S1.fifo_conf.tx_fifo_mod          = 1;
  I2S1.fifo_conf.tx_fifo_mod          = 1;
  I2S1.fifo_conf.tx_data_num          = 32;
  I2S1.fifo_conf.dscr_en              = 1;

  I2S1.conf1.val           = 0;
  I2S1.conf1.tx_stop_en    = 0;
  I2S1.conf1.tx_pcm_bypass = 1;

  I2S1.conf_chan.val         = 0;
  I2S1.conf_chan.tx_chan_mod = 1;

  I2S1.conf.tx_right_first = 0;

  I2S1.timing.val = 0;

  // Reset AHB interface of DMA
  I2S1.lc_conf.ahbm_rst      = 1;
  I2S1.lc_conf.ahbm_fifo_rst = 1;
  I2S1.lc_conf.ahbm_rst      = 0;
  I2S1.lc_conf.ahbm_fifo_rst = 0;

  // Start DMA
  I2S1.lc_conf.val = I2S_OUT_DATA_BURST_EN;// | I2S_OUTDSCR_BURST_EN;
  I2S1.out_link.addr = (uint32_t)m_dma_descriptor;
  I2S1.int_clr.val = 0xFFFFFFFF;
  I2S1.out_link.start = 1;
  I2S1.conf.tx_start  = 1;
}

void DiManager::clear() {
    for (int g = 0; g < ACT_LINES; g++) {
        std::vector<DiPrimitive*> * vp = &m_groups[g];
        vp->clear();
    }

    for (int i = FIRST_PRIMITIVE_ID; i <= LAST_PRIMITIVE_ID; i++) {
      if (m_primitives[i]) {
        delete m_primitives[i];
        m_primitives[i] = NULL;
      }
    }
    m_primitives[ROOT_PRIMITIVE_ID]->clear_child_ptrs();

    heap_caps_free((void*)m_dma_descriptor);
    heap_caps_free((void*)m_video_buffer);
    heap_caps_free((void*)m_front_porch);
    heap_caps_free((void*)m_vertical_sync);
    heap_caps_free((void*)m_back_porch);
}

void DiManager::add_primitive(DiPrimitive* prim, DiPrimitive* parent) {
    auto old_prim = m_primitives[prim->get_id()];
    if (old_prim) {
      delete_primitive(old_prim);
    }

    parent->attach_child(prim);
    while (parent != m_primitives[ROOT_PRIMITIVE_ID] && !(parent->get_flags() & PRIM_FLAG_CLIP_KIDS)) {
      parent = parent->get_parent();
    }
//    prim->compute_absolute_geometry(parent->get_view_x(), parent->get_view_y(),
//      parent->get_view_x_extent(), parent->get_view_y_extent());

/*    if (prim->get_flags() & PRIM_FLAG_PAINT_THIS) {
      int32_t min_group, max_group;
      if (prim->get_vertical_group_range(min_group, max_group)) {
        for (int32_t g = min_group; g <= max_group; g++) {
            m_groups[g].push_back(prim);
        }
      }
    }*/

    m_primitives[prim->get_id()] = prim;
    recompute_primitive(prim, 0, -1, -1);
    prim->generate_instructions();
}

void DiManager::delete_primitive(DiPrimitive* prim) {
  if (prim) {
    if (prim->get_flags() & PRIM_FLAGS_CAN_DRAW) {
      int32_t min_group, max_group;
      if (prim->get_vertical_group_range(min_group, max_group)) {
        for (int32_t g = min_group; g <= max_group; g++) {
          std::vector<DiPrimitive*> * vp = &m_groups[g];
          auto position2 = std::find(vp->begin(), vp->end(), prim);
          if (position2 != vp->end()) {
            vp->erase(position2);
          }
        }
      }
    }

    prim->get_parent()->detach_child(prim);
    DiPrimitive* child = prim->get_first_child();
    while (child) {
      DiPrimitive* next = child->get_next_sibling();
      delete_primitive(child);
      child = next;
    }

    m_primitives[prim->get_id()] = NULL;
    delete prim;
  }
}

void DiManager::recompute_primitive(DiPrimitive* prim, uint8_t old_flags,
                                    int32_t old_min_group, int32_t old_max_group) {
  auto parent = prim->get_parent();
  prim->compute_absolute_geometry(parent->get_view_x(), parent->get_view_y(),
    parent->get_view_x_extent(), parent->get_view_y_extent());

  bool old_use_groups = (old_min_group >= 0);
  bool new_use_groups = false;
  int32_t new_min_group = -1;
  int32_t new_max_group = -1;
  if (prim->get_flags() & PRIM_FLAG_PAINT_THIS) {
    new_use_groups = prim->get_vertical_group_range(new_min_group, new_max_group);
  }
  
  if (old_use_groups) {
    if (new_use_groups) {
      // Adjust which groups primitive is in
      //
      // There are several (vertical) cases:
      // 1. New groups fully above old groups.
      // 2. New groups cross first old group, but not last old group.
      // 3. New groups fully within old groups.
      // 4. New groups cross last old group, but not first old group.
      // 5. New groups fully below old groups.
      // 6. Old groups fully within new groups.

      if (old_min_group < new_min_group) {
        // Remove primitive from old groups that are above new groups
        int32_t end = MIN((old_max_group+1), new_min_group);
        for (int32_t g = old_min_group; g < end; g++) {
          std::vector<DiPrimitive*> * vp = &m_groups[g];
          auto position2 = std::find(vp->begin(), vp->end(), prim);
          if (position2 != vp->end()) {
            vp->erase(position2);
          }
        }
      }

      if (old_max_group > new_max_group) {
        // Remove primitive from old groups that are below new groups
        int32_t begin = MAX((new_max_group+1), old_min_group);
        for (int32_t g = begin; g <= old_max_group; g++) {
          std::vector<DiPrimitive*> * vp = &m_groups[g];
          auto position2 = std::find(vp->begin(), vp->end(), prim);
          if (position2 != vp->end()) {
            vp->erase(position2);
          }
        }
      }

      prim->add_flags(PRIM_FLAGS_CAN_DRAW);
      prim->generate_instructions();
    } else {
      // Just remove primitive from old groups
      for (int32_t g = old_min_group; g <= old_max_group; g++) {
        std::vector<DiPrimitive*> * vp = &m_groups[g];
        auto position2 = std::find(vp->begin(), vp->end(), prim);
        if (position2 != vp->end()) {
          vp->erase(position2);
        }
      }
      prim->remove_flags(PRIM_FLAGS_CAN_DRAW);
      prim->delete_instructions();
    }
  } else {
    if (new_use_groups) {
      // Just place primitive into new groups
      for (int32_t g = new_min_group; g <= new_max_group; g++) {
        std::vector<DiPrimitive*> * vp = &m_groups[g];
        vp->push_back(prim);
      }
      prim->add_flags(PRIM_FLAGS_CAN_DRAW);
      prim->generate_instructions();
    } else {
      prim->remove_flags(PRIM_FLAGS_CAN_DRAW);
      prim->delete_instructions();
    }
  }
}

DiPrimitive* DiManager::finish_create(uint16_t id, uint8_t flags, DiPrimitive* prim, DiPrimitive* parent_prim) {
    prim->set_id(id);
    prim->set_flags(flags);
    add_primitive(prim, parent_prim);
    return prim;
}

DiPrimitive* DiManager::create_point(uint16_t id, uint16_t parent, uint8_t flags,
                            int32_t x, int32_t y, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    DiPrimitive* prim = new DiSetPixel(x, y, color);

    return finish_create(id, flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_line(uint16_t id, uint16_t parent, uint8_t flags,
                            int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    DiPrimitive* prim;
    if (x1 == x2) {
        if (y1 == y2) {
            prim = new DiSetPixel(x1, y1, color);
        } else if (y1 < y2) {
            auto line = new DiVerticalLine();
            line->init_params(x1, y1, y2 - y1 + 1, color);
            prim = line;
        } else {
            auto line = new DiVerticalLine();
            line->init_params(x1, y2, y1 - y2 + 1, color);
            prim = line;
        }
    } else if (x1 < x2) {
        if (y1 == y2) {
            auto line = new DiHorizontalLine();
            line->init_params(x1, y1, x2 - x1 + 1, color);
            prim = line;
        } else if (y1 < y2) {
            if (y2 - y1 == x2 - x1) {
                auto line = new DiDiagonalRightLine();
                line->init_params(x1, y1, x2 - x1 + 1, color);
                prim = line;
            } else {
                auto line = new DiGeneralLine();
                line->init_params(x1, y1, x2, y2, color);
                prim = line;
            }
        } else {
            if (y2 - y1 == x2 - x1) {
                auto line = new DiDiagonalLeftLine();
                line->init_params(x2, y1, x2 - x1 + 1, color);
                prim = line;
            } else {
                auto line = new DiGeneralLine();
                line->init_params(x1, y1, x2, y2, color);
                prim = line;
            }
        }
    } else {
        if (y1 == y2) {
            auto line = new DiHorizontalLine();
            line->init_params(x2, y1, x1 - x2 + 1, color);
            prim = line;
        } else if (y1 < y2) {
            if (y2 - y1 == x1 - x2) {
                auto line = new DiDiagonalLeftLine();
                line->init_params(x1, y1, x1 - x2 + 1, color);
                prim = line;
            } else {
                auto line = new DiGeneralLine();
                line->init_params(x1, y1, x2, y2, color);
                prim = line;
            }
        } else {
            if (y2 - y1 == x1 - x2) {
                auto line = new DiDiagonalRightLine();
                line->init_params(x2, y1, x1 - x2 + 1, color);
                prim = line;
            } else {
                auto line = new DiGeneralLine();
                line->init_params(x1, y1, x2, y2, color);
                prim = line;
            }
        }
    }

    return finish_create(id, flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_solid_rectangle(uint16_t id, uint16_t parent, uint8_t flags,
                            int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiSolidRectangle();
    prim->init_params(x, y, width, height, color);

    return finish_create(id, flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_triangle(uint16_t id, uint16_t parent, uint8_t flags,
                            int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                            int32_t x3, int32_t y3, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiGeneralLine();
    prim->init_params(x1, y1, x2, y2, x3, y3, color);

    return finish_create(id, flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_solid_triangle(uint16_t id, uint16_t parent, uint8_t flags,
                            int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                            int32_t x3, int32_t y3, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiGeneralLine();
    prim->init_params(x1, y1, x2, y2, x3, y3, color);

    return finish_create(id, flags, prim, parent_prim);
}

DiTileMap* DiManager::create_tile_map(uint16_t id, uint16_t parent, uint8_t flags,
                            int32_t screen_width, int32_t screen_height,
                            uint32_t bitmaps, uint32_t columns, uint32_t rows,
                            uint32_t width, uint32_t height) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto hscroll = ((flags & PRIM_FLAG_H_SCROLL) != 0);
    DiTileMap* tile_map =
      new DiTileMap(screen_width, screen_height, bitmaps, columns, rows, width, height, hscroll);

    finish_create(id, flags, tile_map, parent_prim);
    return tile_map;
}

DiTerminal* DiManager::create_terminal(uint16_t id, uint16_t parent, uint8_t flags,
                            uint32_t x, uint32_t y, uint32_t codes, uint32_t columns, uint32_t rows,
                            uint8_t fg_color, uint8_t bg_color, const uint8_t* font) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    DiTerminal* terminal = new DiTerminal(x, y, codes, columns, rows, fg_color, bg_color, font);

    finish_create(id, flags, terminal, parent_prim);
    m_terminal = terminal;
    return terminal;
}

void IRAM_ATTR DiManager::run() {
  initialize();
  loop();
  clear();
}

void IRAM_ATTR DiManager::loop() {
  uint32_t current_line_index = 0;//NUM_ACTIVE_BUFFERS * NUM_LINES_PER_BUFFER;
  uint32_t current_buffer_index = 0;
  LoopState loop_state = LoopState::NearNewFrameStart;

  while (true) {
    uint32_t descr_addr = (uint32_t) I2S1.out_link_dscr;
    uint32_t descr_index = (descr_addr - (uint32_t)m_dma_descriptor) / sizeof(lldesc_t);
    if (descr_index <= ACT_BUFFERS_WRITTEN) {
      //uint32_t dma_line_index = descr_index * NUM_LINES_PER_BUFFER;
      uint32_t dma_buffer_index = descr_index & (NUM_ACTIVE_BUFFERS-1);

      // Draw enough lines to stay ahead of DMA.
      while (current_line_index < ACT_LINES && current_buffer_index != dma_buffer_index) {
        volatile DiVideoBuffer* vbuf = &m_video_buffer[current_buffer_index];
        draw_primitives(vbuf->get_buffer_ptr_0(), current_line_index);
        draw_primitives(vbuf->get_buffer_ptr_1(), ++current_line_index);

        ++current_line_index;
        if (++current_buffer_index >= NUM_ACTIVE_BUFFERS) {
          current_buffer_index = 0;
        }
      }

      loop_state = LoopState::WritingActiveLines;

      while (ESPSerial.available() > 0) {
        store_character(ESPSerial.read());
      }

      (*m_on_lines_painted_cb)();

    } else if (loop_state == LoopState::WritingActiveLines) {
      process_stored_characters();
      while (ESPSerial.available() > 0) {
        process_character(ESPSerial.read());
      }
      (*m_on_vertical_blank_cb)();

      loop_state = LoopState::ProcessingIncomingData;
      
    } else if (descr_index >= DMA_TOTAL_DESCR - DMA_ACT_LINES - 1) {
      // Prepare the start of the next frame.
      for (current_line_index = 0, current_buffer_index = 0;
            current_buffer_index < NUM_ACTIVE_BUFFERS;
            current_line_index++, current_buffer_index++) {
        volatile DiVideoBuffer* vbuf = &m_video_buffer[current_buffer_index];
        draw_primitives(vbuf->get_buffer_ptr_0(), current_line_index);
        draw_primitives(vbuf->get_buffer_ptr_1(), ++current_line_index);
      }

      loop_state = LoopState::NearNewFrameStart;
      current_line_index = 0;
      current_buffer_index = 0;

    } else if (loop_state == LoopState::ProcessingIncomingData) {
      // Keep handling incoming characters
      if (ESPSerial.available() > 0) {
        process_character(ESPSerial.read());
      }

    } else {
      // Keep storing incoming characters
      if (ESPSerial.available() > 0) {
        store_character(ESPSerial.read());
      }
    }
  }
}

void IRAM_ATTR DiManager::draw_primitives(volatile uint32_t* p_scan_line, uint32_t line_index) {
  std::vector<DiPrimitive*> * vp = &m_groups[line_index];
  for (auto prim = vp->begin(); prim != vp->end(); ++prim) {
      (*prim)->paint(p_scan_line, line_index);
  }
}

void DiManager::set_on_vertical_blank_cb(DiVoidCallback callback_fcn) {
  if (callback_fcn) {
    m_on_vertical_blank_cb = callback_fcn;
  } else {
    m_on_vertical_blank_cb = default_on_vertical_blank;
  }
}

void DiManager::set_on_lines_painted_cb(DiVoidCallback callback_fcn) {
  if (callback_fcn) {
    m_on_lines_painted_cb = callback_fcn;
  } else {
    m_on_lines_painted_cb = default_on_lines_painted;
  }
}

void DiManager::init_dma_descriptor(volatile DiVideoScanLine* vline, uint32_t descr_index) {
  volatile lldesc_t * dd = &m_dma_descriptor[descr_index];

  if (descr_index == 0) {
    m_dma_descriptor[DMA_TOTAL_DESCR - 1].qe.stqe_next = (lldesc_t*)dd;
  } else {
    m_dma_descriptor[descr_index - 1].qe.stqe_next = (lldesc_t*)dd;
  }

  dd->sosf = dd->offset = dd->eof = 0;
  dd->owner = 1;
  dd->size = vline->get_buffer_size();
  dd->length = vline->get_buffer_size();
  dd->buf = (uint8_t volatile *)vline->get_buffer_ptr();
}

void DiManager::init_dma_descriptor(volatile DiVideoBuffer* vbuf, uint32_t descr_index) {
  volatile lldesc_t * dd = &m_dma_descriptor[descr_index];

  if (descr_index == 0) {
    m_dma_descriptor[DMA_TOTAL_DESCR - 1].qe.stqe_next = (lldesc_t*)dd;
  } else {
    m_dma_descriptor[descr_index - 1].qe.stqe_next = (lldesc_t*)dd;
  }

  dd->sosf = dd->offset = dd->eof = 0;
  dd->owner = 1;
  dd->size = vbuf->get_buffer_size();
  dd->length = vbuf->get_buffer_size();
  dd->buf = (uint8_t volatile *)vbuf->get_buffer_ptr_0();
}

void DiManager::store_character(uint8_t character) {
  if (m_num_buffer_chars < INCOMING_DATA_BUFFER_SIZE) {
    m_incoming_data[m_next_buffer_write++] = character;
    if (m_next_buffer_write >= INCOMING_DATA_BUFFER_SIZE) {
      m_next_buffer_write = 0;
    }
    m_num_buffer_chars++;
  }
}

void DiManager::store_string(const uint8_t* string) {
  while (uint8_t character = *string++) {
    store_character(character);
  }
}

void DiManager::process_stored_characters() {
  while (m_num_buffer_chars > 0) {
    bool rc = process_character(m_incoming_data[m_next_buffer_read++]);
    if (m_next_buffer_read >= INCOMING_DATA_BUFFER_SIZE) {
      m_next_buffer_read = 0;
    }
    m_num_buffer_chars--;
    //if (!rc && !m_num_buffer_chars) {
    //  break; // need to wait for more data
    //}
  }
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
bool DiManager::process_character(uint8_t character) {
  if (m_num_command_chars) {
    switch (m_incoming_command[0]) {
      case 0x11: return ignore_cmd(character, 2);
      case 0x17: return handle_udg_sys_cmd(character);
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
      case 0x11: return ignore_cmd(character, 2); // set color
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

void DiManager::process_string(const uint8_t* string) {
  while (uint8_t character = *string++) {
    if (!process_character(character)) {
      break;
    }
  }
}

bool DiManager::ignore_cmd(uint8_t character, uint8_t len) {
  m_incoming_command[m_num_command_chars++] = character;
  if (m_num_command_chars >= len) {
    m_num_command_chars = 0;
    return true;
  }
  return false;
}

bool DiManager::define_graphics_viewport(uint8_t character) {
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

bool DiManager::define_text_viewport(uint8_t character) {
  m_incoming_command[m_num_command_chars++] = character;
  if (m_num_command_chars >= 5) {
      uint8_t left = get_param_8(1);
      uint8_t bottom = get_param_8(2);
      uint8_t right = get_param_8(3);
      uint8_t top = get_param_8(4);
      m_num_command_chars = 0;
      return true;
  }
  return false;
}

void DiManager::report(uint8_t character) {
  write_character('[');
  write_character(to_hex(character >> 4));
  write_character(to_hex(character & 0xF));
  write_character(']');
}

uint8_t DiManager::to_hex(uint8_t value) {
  if (value < 10) {
    return value + 0x30; // '0'
  } else {
    return value - 10 + 0x41; // 'A'
  }
}

uint8_t DiManager::peek_into_buffer() {
  return m_incoming_data[m_next_buffer_read];
}

uint8_t DiManager::read_from_buffer() {
  if (m_num_buffer_chars) {
    uint8_t character = m_incoming_data[m_next_buffer_read++];
    if (m_next_buffer_read >= INCOMING_DATA_BUFFER_SIZE) {
      m_next_buffer_read = 0;
    }
    m_num_buffer_chars--;
    return character;
  } else {
    return 0;
  }
}

void DiManager::skip_from_buffer() {
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
bool DiManager::handle_udg_sys_cmd(uint8_t character) {
  m_incoming_command[m_num_command_chars++] = character;
  if (m_num_command_chars >= 2 && get_param_8(1) == 30) {
    return handle_otf_cmd();
  }
  if (m_num_command_chars >= 3) {
    switch (m_incoming_command[2]) {

      // VDU 23, 0, &80, b: General poll
      case VDP_GP: /*0x80*/ {
        if (m_num_command_chars == 4) {
          uint8_t echo = get_param_8(3);
          send_general_poll(echo);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 0, &81, n: Set the keyboard locale (0=UK, 1=US, etc)
      case VDP_KEYCODE: /*0x81*/ {
        if (m_num_command_chars == 4) {
          uint8_t region = get_param_8(3);
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

      default: {
        m_num_command_chars = 0;
        return true;
      }
    }
  }
  return false;
}

/*
800x600x64 On-the-Fly Command Set:
VDU 23, 30, 0, id; flags: [6] Set flags for primitive
VDU 23, 30, 1, id; x; y;: [9] Move primitive: absolute
VDU 23, 30, 2, id; x; y;: [9] Move primitive: relative
VDU 23, 30, 3, id;: [5] Delete primitive
VDU 23, 30, 4, id; pid; flags, x; y; c: [13] Create primitive: Point
VDU 23, 30, 5, id; pid; flags, x1; y1; x2; y2; c: [17] Create primitive: Line
VDU 23, 30, 6, id; pid; flags, x1; y1; x2; y2; x3; y3; c: [21] Create primitive: Triangle Outline
VDU 23, 30, 7, id; pid; flags, x1; y1; x2; y2; x3; y3; c: [21] Create primitive: Solid Triangle
VDU 23, 30, 8, id; pid; flags, x; y; w; h; c: [17] Create primitive: Rectangle Outline
VDU 23, 30, 9, id; pid; flags, x; y; w; h; c: [17] Create primitive: Solid Rectangle
VDU 23, 30, 10, id; pid; flags, x; y; w; h; c: [17] Create primitive: Ellipse Outline
VDU 23, 30, 11, id; pid; flags, x; y; w; h; c: [17] Create primitive: Solid Ellipse
VDU 23, 30, 12, id; pid; flags, cols; rows; bitmaps, w; h;: [17] Create primitive: Tile Map
VDU 23, 30, 13, id; pid; flags, w; h;: [12] Create primitive: Solid Bitmap
VDU 23, 30, 14, id; pid; flags, w; h;: [12] Create primitive: Masked Bitmap
VDU 23, 30, 15, id; pid; flags, w; h; c: [13] Create primitive: Transparent Bitmap
VDU 23, 30, 16, id; pid; flags, x; y;: [12] Create primitive: Group
VDU 23, 30, 17, id; x; y; s; h;: [13] Move & slice solid bitmap: absolute
VDU 23, 30, 18, id; x; y; s; h;: [13] Move & slice masked bitmap: absolute
VDU 23, 30, 19, id; x; y; s; h;: [13] Move & slice transparent bitmap: absolute
VDU 23, 30, 20, id; x; y; s; h;: [13] Move & slice solid bitmap: relative
VDU 23, 30, 21, id; x; y; s; h;: [13] Move & slice masked bitmap: relative
VDU 23, 30, 22, id; x; y; s; h;: [13] Move & slice transparent bitmap: relative
VDU 23, 30, 23, id; x; y; c: [10] Set solid bitmap pixel
VDU 23, 30, 24, id; x; y; c: [10] Set masked bitmap pixel
VDU 23, 30, 25, id; x; y; c: [10] Set transparent bitmap pixel
VDU 23, 30, 26, id; x; y; n; c0, c1, c2, ...: [11+n] Set solid bitmap pixels
VDU 23, 30, 27, id; x; y; n; c0, c1, c2, ...: [11+n] Set masked bitmap pixels
VDU 23, 30, 28, id; x; y; n; c0, c1, c2, ...: [11+n] Set transparent bitmap pixels
VDU 23, 30, 29, id; col; row; bi: [10] Set bitmap index for tile in tile map
VDU 23, 30, 30, id; bi, x; y; c: [11] Set bitmap pixel in tile map
VDU 23, 30, 31, id; bi, x; y; n; c0, c1, c2, ...: [12+n] Set bitmap pixels in tile map
*/
bool DiManager::handle_otf_cmd() {
  if (m_num_command_chars >= 5) {
    int16_t p = get_param_16(3); // get primitive index number
    switch (m_incoming_command[2]) {

      // VDU 23, 30, 0, id; flags: [6] Set flags for primitive
      case 0: {
        if (m_num_command_chars == 6) {
          auto id = get_param_16(3);
          auto flags = get_param_8(5);
          set_primitive_flags(id, flags);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 1, id; x; y;: [9] Move primitive: absolute
      case 1: {
        if (m_num_command_chars == 9) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          move_primitive_absolute(id, x, y);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 2, id; x; y;: [9] Move primitive: relative
      case 2: {
        if (m_num_command_chars == 9) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          move_primitive_relative(id, x, y);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 3, id;: [5] Delete primitive
      case 3: {
        if (m_num_command_chars == 5) {
          auto id = get_param_16(3);
          delete_primitive(id);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 4, id; pid; flags, x; y; c: [13] Create primitive: Point
      case 4: {
        if (m_num_command_chars == 13) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto x = get_param_16(8);
          auto y = get_param_16(10);
          auto c = get_param_8(12);
          create_point(id, pid, flags, x, y, c);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 5, id; pid; flags, x1; y1; x2; y2; c: [17] Create primitive: Line
      case 5: {
        if (m_num_command_chars == 17) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto x1 = get_param_16(8);
          auto y1 = get_param_16(10);
          auto x2 = get_param_16(12);
          auto y2 = get_param_16(14);
          auto c = get_param_8(16);
          create_line(id, pid, flags, x1, y1, x2, y2, c);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 6, id; pid; flags, x1; y1; x2; y2; x3; y3; c: [21] Create primitive: Triangle Outline
      case 6: {
        if (m_num_command_chars == 21) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto x1 = get_param_16(8);
          auto y1 = get_param_16(10);
          auto x2 = get_param_16(12);
          auto y2 = get_param_16(14);
          auto x3 = get_param_16(16);
          auto y3 = get_param_16(18);
          auto c = get_param_8(20);
          create_triangle(id, pid, flags, x1, y1, x2, y2, x3, y3, c);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 7, id; pid; flags, x1; y1; x2; y2; x3; y3; c: [21] Create primitive: Solid Triangle
      case 7: {
        if (m_num_command_chars == 21) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto x1 = get_param_16(8);
          auto y1 = get_param_16(10);
          auto x2 = get_param_16(12);
          auto y2 = get_param_16(14);
          auto x3 = get_param_16(16);
          auto y3 = get_param_16(18);
          auto c = get_param_8(20);
          create_solid_triangle(id, pid, flags, x1, y1, x2, y2, x3, y3, c);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 8, id; pid; flags, x; y; w; h; c: [17] Create primitive: Rectangle Outline
      case 8: {
        if (m_num_command_chars == 17) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto x = get_param_16(8);
          auto y = get_param_16(10);
          auto w = get_param_16(12);
          auto h = get_param_16(14);
          auto c = get_param_8(16);
          create_rectangle(id, pid, flags, x, y, w, h, c);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 9, id; pid; flags, x; y; w; h; c: [17] Create primitive: Solid Rectangle
      case 9: {
        if (m_num_command_chars == 17) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto x = get_param_16(8);
          auto y = get_param_16(10);
          auto w = get_param_16(12);
          auto h = get_param_16(14);
          auto c = get_param_8(16);
          create_solid_rectangle(id, pid, flags, x, y, w, h, c);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 10, id; pid; flags, x; y; w; h; c: [17] Create primitive: Ellipse Outline
      case 10: {
        if (m_num_command_chars == 17) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto x = get_param_16(8);
          auto y = get_param_16(10);
          auto w = get_param_16(12);
          auto h = get_param_16(14);
          auto c = get_param_8(16);
          create_ellipse(id, pid, flags, x, y, w, h, c);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 11, id; pid; flags, x; y; w; h; c: [17] Create primitive: Solid Ellipse
      case 11: {
        if (m_num_command_chars == 17) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto x = get_param_16(8);
          auto y = get_param_16(10);
          auto w = get_param_16(12);
          auto h = get_param_16(14);
          auto c = get_param_8(16);
          create_solid_ellipse(id, pid, flags, x, y, w, h, c);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 12, id; pid; flags, cols; rows; bitmaps, w; h;: [17] Create primitive: Tile Map
      case 12: {
        if (m_num_command_chars == 17) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto cols = get_param_16(8);
          auto rows = get_param_16(10);
          auto bitmaps = get_param_8(12);
          auto w = get_param_16(13);
          auto h = get_param_16(15);
          create_tile_map(id, pid, flags, ACT_PIXELS, ACT_LINES, bitmaps, cols, rows, w, h);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 13, id; pid; flags, w; h;: [12] Create primitive: Solid Bitmap
      case 13: {
        if (m_num_command_chars == 12) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto w = get_param_16(8);
          auto h = get_param_16(10);
          create_solid_bitmap(id, pid, flags, w, h);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 14, id; pid; flags, w; h;: [12] Create primitive: Masked Bitmap
      case 14: {
        if (m_num_command_chars == 12) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto w = get_param_16(8);
          auto h = get_param_16(10);
          create_masked_bitmap(id, pid, flags, w, h);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 15, id; pid; flags, w; h; c: [13] Create primitive: Transparent Bitmap
      case 15: {
        if (m_num_command_chars == 13) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto w = get_param_16(8);
          auto h = get_param_16(10);
          auto c = get_param_8(12);
          create_transparent_bitmap(id, pid, flags, w, h, c);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 16, id; pid; flags, x; y;: [12] Create primitive: Group
      case 16: {
        if (m_num_command_chars == 12) {
          auto id = get_param_16(3);
          auto pid = get_param_16(5);
          auto flags = get_param_8(7);
          auto x = get_param_16(8);
          auto y = get_param_16(10);
          create_primitive_group(id, pid, flags, x, y);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 17, id; x; y; s; h;: [13] Move & slice solid bitmap: absolute
      case 17: {
        if (m_num_command_chars == 13) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto s = get_param_16(9);
          auto h = get_param_16(11);
          slice_solid_bitmap_absolute(id, x, y, s, h);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 18, id; x; y; s; h;: [13] Move & slice masked bitmap: absolute
      case 18: {
        if (m_num_command_chars == 13) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto s = get_param_16(9);
          auto h = get_param_16(11);
          slice_masked_bitmap_absolute(id, x, y, s, h);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 19, id; x; y; s; h;: [13] Move & slice transparent bitmap: absolute
      case 19: {
        if (m_num_command_chars == 13) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto s = get_param_16(9);
          auto h = get_param_16(11);
          slice_transparent_bitmap_absolute(id, x, y, s, h);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 20, id; x; y; s; h;: [13] Move & slice solid bitmap: relative
      case 20: {
        if (m_num_command_chars == 13) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto s = get_param_16(9);
          auto h = get_param_16(11);
          slice_solid_bitmap_relative(id, x, y, s, h);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 21, id; x; y; s; h;: [13] Move & slice masked bitmap: relative
      case 21: {
        if (m_num_command_chars == 13) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto s = get_param_16(9);
          auto h = get_param_16(11);
          slice_masked_bitmap_relative(id, x, y, s, h);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 22, id; x; y; s; h;: [13] Move & slice transparent bitmap: relative
      case 22: {
        if (m_num_command_chars == 13) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto s = get_param_16(9);
          auto h = get_param_16(11);
          slice_transparent_bitmap_relative(id, x, y, s, h);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 23, id; x; y; c: [10] Set solid bitmap pixel
      case 23: {
        if (m_num_command_chars == 10) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto c = get_param_8(9);
          set_solid_bitmap_pixel(id, x, y, c, 0);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 24, id; x; y; c: [10] Set masked bitmap pixel
      case 24: {
        if (m_num_command_chars == 10) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto c = get_param_8(9);
          set_masked_bitmap_pixel(id, x, y, c, 0);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 25, id; x; y; c: [10] Set transparent bitmap pixel
      case 25: {
        if (m_num_command_chars == 10) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto c = get_param_8(9);
          set_transparent_bitmap_pixel(id, x, y, c, 0);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 26, id; x; y; n; c0, c1, c2, ...: [11+n] Set solid bitmap pixels
      case 26: {
        if (m_num_command_chars >= 12) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto n = get_param_16(9);
          auto c = get_param_8(11);
          set_solid_bitmap_pixel(id, x, y, c, m_command_data_index);
          if (++m_command_data_index >= n) {
            m_num_command_chars = 0;
            return true;
          } else {
            m_num_command_chars = 11;
          }
        } else if (m_num_command_chars == 5) {
          m_command_data_index = 0;
        }
      } break;

      // VDU 23, 30, 27, id; x; y; n; c0, c1, c2, ...: [11+n] Set masked bitmap pixels
      case 27: {
        if (m_num_command_chars >= 12) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto n = get_param_16(9);
          auto c = get_param_8(11);
          set_masked_bitmap_pixel(id, x, y, c, m_command_data_index);
          if (++m_command_data_index >= n) {
            m_num_command_chars = 0;
            return true;
          } else {
            m_num_command_chars = 11;
          }
        } else if (m_num_command_chars == 5) {
          m_command_data_index = 0;
        }
      } break;

      // VDU 23, 30, 28, id; x; y; n; c0, c1, c2, ...: [11+n] Set transparent bitmap pixels
      case 28: {
        if (m_num_command_chars >= 12) {
          auto id = get_param_16(3);
          auto x = get_param_16(5);
          auto y = get_param_16(7);
          auto n = get_param_16(9);
          auto c = get_param_8(11);
          set_transparent_bitmap_pixel(id, x, y, c, m_command_data_index);
          if (++m_command_data_index >= n) {
            m_num_command_chars = 0;
            return true;
          } else {
            m_num_command_chars = 11;
          }
        } else if (m_num_command_chars == 5) {
          m_command_data_index = 0;
        }
      } break;

      // VDU 23, 30, 29, id; col; row; bi: [10] Set bitmap index for tile in tile map
      case 29: {
        if (m_num_command_chars == 10) {
          auto id = get_param_16(3);
          auto col = get_param_16(5);
          auto row = get_param_16(7);
          auto bi = get_param_8(9);
          set_tile_bitmap_index(id, col, row, bi);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 30, id; bi, x; y; c: [11] Set bitmap pixel in tile map
      case 30: {
        if (m_num_command_chars == 11) {
          auto id = get_param_16(3);
          auto bi = get_param_8(5);
          auto x = get_param_16(6);
          auto y = get_param_16(8);
          auto c = get_param_8(10);
          set_tile_bitmap_pixel(id, bi, x, y, c, 0);
          m_num_command_chars = 0;
          return true;
        }
      } break;

      // VDU 23, 30, 31, id; bi, x; y; n; c0, c1, c2, ...: [12+n] Set bitmap pixels in tile map
      case 31: {
        if (m_num_command_chars >= 13) {
          auto id = get_param_16(3);
          auto bi = get_param_8(5);
          auto x = get_param_16(6);
          auto y = get_param_16(8);
          auto n = get_param_16(10);
          auto c = get_param_8(12);
          set_tile_bitmap_pixel(id, bi, x, y, c, m_command_data_index);
          if (++m_command_data_index >= n) {
            m_num_command_chars = 0;
            return true;
          } else {
            m_num_command_chars = 10;
          }
        } else if (m_num_command_chars == 5) {
          m_command_data_index = 0;
        }
      } break;

    }
  }
  return false;
}

void DiManager::clear_screen() {
  if (m_terminal) {
    m_terminal->clear_screen();
  }
}

void DiManager::move_cursor_left() {
  if (m_terminal) {
    m_terminal->move_cursor_left();
  }
}

void DiManager::move_cursor_right() {
  if (m_terminal) {
    m_terminal->move_cursor_right();
  }
}

void DiManager::move_cursor_down() {
  if (m_terminal) {
    m_terminal->move_cursor_down();
  }
}

void DiManager::move_cursor_up() {
  if (m_terminal) {
    m_terminal->move_cursor_up();
  }
}

void DiManager::move_cursor_home() {
  if (m_terminal) {
    m_terminal->move_cursor_home();
  }
}

void DiManager::move_cursor_boln() {
  if (m_terminal) {
    m_terminal->move_cursor_boln();
  }
}

void DiManager::do_backspace() {
  if (m_terminal) {
    m_terminal->do_backspace();
  }
}

bool DiManager::move_cursor_tab(uint8_t character) {
  m_incoming_command[m_num_command_chars++] = character;
  if (m_num_command_chars >= 3) {
    if (m_terminal) {
      uint8_t x = get_param_8(1);
      uint8_t y = get_param_8(2);
      m_terminal->move_cursor_tab(x, y);
    }
    m_num_command_chars = 0;
    return true;
  }
  return false;
}

uint8_t DiManager::read_character(int16_t x, int16_t y) {
  if (m_terminal) {
    return m_terminal->read_character(x, y);
  } else {
    return 0;
  }
}

void DiManager::write_character(uint8_t character) {
  if (m_terminal) {
    m_terminal->write_character(character);
  }
}

uint8_t DiManager::get_param_8(uint32_t index) {
  return m_incoming_command[index];
}

int16_t DiManager::get_param_16(uint32_t index) {
  return (int16_t)((((uint16_t)m_incoming_command[index+1]) << 8) | m_incoming_command[index]);
}

// Send the cursor position back to MOS
//
void DiManager::send_cursor_position() {
  uint16_t column = 0;
  uint16_t row = 0;
  if (m_terminal) {
    m_terminal->get_position(column, row);
  }
	byte packet[] = {
		(byte) column,
		(byte) row
	};
	send_packet(PACKET_CURSOR, sizeof packet, packet);	
}

// Send a character back to MOS
//
void DiManager::send_screen_char(int16_t x, int16_t y) {
	uint8_t c = read_character(x, y);
	byte packet[] = {
		c
	};
	send_packet(PACKET_SCRCHAR, sizeof packet, packet);
}

// Send a pixel value back to MOS
//
void DiManager::send_screen_pixel(int16_t x, int16_t y) {
	byte packet[] = {
		0,	// R
		0,  // G
		0,  // B
		0 	// There is no palette in this mode.
	};
	send_packet(PACKET_SCRPIXEL, sizeof packet, packet);	
}

// Send MODE information (screen details)
//
void DiManager::send_mode_information() {
	byte packet[] = {
		ACT_PIXELS & 0xFF,	 				// Width in pixels (L)
		(ACT_PIXELS >> 8) & 0xFF,		// Width in pixels (H)
		ACT_LINES & 0xFF,						// Height in pixels (L)
		(ACT_LINES >> 8) & 0xFF,		// Height in pixels (H)
		(ACT_PIXELS / 8),					  // Width in characters (byte)
		(ACT_LINES / 8),					  // Height in characters (byte)
		64,						              // Colour depth
		(uint8_t)videoMode          // The video mode number
	};
	send_packet(PACKET_MODE, sizeof packet, packet);
}

// Send a general poll
//
void DiManager::send_general_poll(uint8_t b) {
	byte packet[] = {
		b 
	};
	send_packet(PACKET_GP, sizeof packet, packet);
	initialised = true;	
}

void DiManager::set_primitive_flags(uint16_t id, uint8_t flags) {
  DiPrimitive* prim; if (!(prim = (DiPrimitive*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  auto chg_flags = flags & PRIM_FLAGS_CHANGEABLE;
  auto new_flags = (old_flags & ~PRIM_FLAGS_CHANGEABLE) | chg_flags;
  prim->set_flags(new_flags);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group);
}

void DiManager::move_primitive_absolute(uint16_t id, int32_t x, int32_t y) {
  DiPrimitive* prim; if (!(prim = (DiPrimitive*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  prim->set_relative_position(x, y);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group);
}

void DiManager::move_primitive_relative(uint16_t id, int32_t x, int32_t y) {
  DiPrimitive* prim; if (!(prim = (DiPrimitive*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  auto x2 = prim->get_relative_x() + x;
  auto y2 = prim->get_relative_y() + y;
  prim->set_relative_position(x2, y2);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group);  
}

void DiManager::delete_primitive(uint16_t id) {
  DiPrimitive* prim; if (!(prim = (DiPrimitive*)get_safe_primitive(id))) return;
  delete_primitive(prim);  
}

DiPrimitive* DiManager::create_rectangle(uint16_t id, uint16_t parent, uint8_t flags,
                        int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiRectangle();
    prim->init_params(x, y, width, height, color);

    return finish_create(id, flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_ellipse(uint16_t id, uint16_t parent, uint8_t flags,
                        int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiEllipse();
    prim->init_params(x, y, width, height, color);

    return finish_create(id, flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_solid_ellipse(uint16_t id, uint16_t parent, uint8_t flags,
                        int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiSolidEllipse();
    prim->init_params(x, y, width, height, color);

    return finish_create(id, flags, prim, parent_prim);
}

ScrollMode get_scroll_mode_from_flags(uint8_t flags) {
    if (flags & PRIM_FLAG_H_SCROLL) {
      if (flags & PRIM_FLAG_V_SCROLL) {
        return ScrollMode::BOTH;
      } else {
        return ScrollMode::HORIZONTAL;
      }
    } else if (flags & PRIM_FLAG_V_SCROLL) {
      return ScrollMode::VERTICAL;
    } else {
      return ScrollMode::NONE;
    }
}

DiOpaqueBitmap* DiManager::create_solid_bitmap(uint16_t id, uint16_t parent, uint8_t flags,
                        uint32_t width, uint32_t height) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto scroll_mode = get_scroll_mode_from_flags(flags);
    DiOpaqueBitmap* prim = new DiOpaqueBitmap(width, height, scroll_mode);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiMaskedBitmap* DiManager::create_masked_bitmap(uint16_t id, uint16_t parent, uint8_t flags,
                        uint32_t width, uint32_t height) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto scroll_mode = get_scroll_mode_from_flags(flags);
    DiMaskedBitmap* prim = new DiMaskedBitmap(width, height, scroll_mode);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiTransparentBitmap* DiManager::create_transparent_bitmap(uint16_t id, uint16_t parent, uint8_t flags,
                        uint32_t width, uint32_t height, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto scroll_mode = get_scroll_mode_from_flags(flags);
    DiTransparentBitmap* prim = new DiTransparentBitmap(width, height, scroll_mode);
    prim->set_transparent_color(color);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiPrimitive* DiManager::create_primitive_group(uint16_t id, uint16_t parent, uint8_t flags,
                        int32_t x, int32_t y) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    DiPrimitive* prim = new DiPrimitive();
    prim->set_relative_position(x, y);

    return finish_create(id, flags, prim, parent_prim);
}

void DiManager::slice_solid_bitmap_absolute(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height) {
  DiOpaqueBitmap* prim; if (!(prim = (DiOpaqueBitmap*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  prim->set_slice_position(x, y, start_line, height);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group);
}

void DiManager::slice_masked_bitmap_absolute(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height) {
  DiMaskedBitmap* prim; if (!(prim = (DiMaskedBitmap*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  prim->set_slice_position(x, y, start_line, height);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group);
}

void DiManager::slice_transparent_bitmap_absolute(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height) {
  DiTransparentBitmap* prim; if (!(prim = (DiTransparentBitmap*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  prim->set_slice_position(x, y, start_line, height);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group);
}

void DiManager::slice_solid_bitmap_relative(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height) {
  DiOpaqueBitmap* prim; if (!(prim = (DiOpaqueBitmap*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  auto x2 = prim->get_relative_x() + x;
  auto y2 = prim->get_relative_y() + y;
  prim->set_slice_position(x, y, start_line, height);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group);
}

void DiManager::slice_masked_bitmap_relative(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height) {
  DiMaskedBitmap* prim; if (!(prim = (DiMaskedBitmap*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  auto x2 = prim->get_relative_x() + x;
  auto y2 = prim->get_relative_y() + y;
  prim->set_slice_position(x, y, start_line, height);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group);
}

void DiManager::slice_transparent_bitmap_relative(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height) {
  DiTransparentBitmap* prim; if (!(prim = (DiTransparentBitmap*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  auto x2 = prim->get_relative_x() + x;
  auto y2 = prim->get_relative_y() + y;
  prim->set_slice_position(x, y, start_line, height);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group); 
}

void DiManager::set_solid_bitmap_pixel(uint16_t id, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiOpaqueBitmap* prim; if (!(prim = (DiOpaqueBitmap*)get_safe_primitive(id))) return;
  int32_t px = x + nth;
  int32_t py = y;
  while (px >= prim->get_width()) {
    px -= prim->get_width();
    py++;
  }
  prim->set_opaque_pixel(px, py, color);
}

void DiManager::set_masked_bitmap_pixel(uint16_t id, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiMaskedBitmap* prim; if (!(prim = (DiMaskedBitmap*)get_safe_primitive(id))) return;
  int32_t px = x + nth;
  int32_t py = y;
  while (px >= prim->get_width()) {
    px -= prim->get_width();
    py++;
  }
  prim->set_masked_pixel(px, py, color);
}

void DiManager::set_transparent_bitmap_pixel(uint16_t id, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiTransparentBitmap* prim; if (!(prim = (DiTransparentBitmap*)get_safe_primitive(id))) return;
  int32_t px = x + nth;
  int32_t py = y;
  while (px >= prim->get_width()) {
    px -= prim->get_width();
    py++;
  }
  prim->set_transparent_pixel(px, py, color);
}

void DiManager::set_tile_bitmap_index(uint16_t id, uint16_t col, uint16_t row, uint8_t bitmap) {
  DiTileMap* prim; if (!(prim = (DiTileMap*)get_safe_primitive(id))) return;
  prim->set_tile(col, row, bitmap);
}

void DiManager::set_tile_bitmap_pixel(uint16_t id, uint8_t bitmap, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiTileMap* prim; if (!(prim = (DiTileMap*)get_safe_primitive(id))) return;
  x += nth;
  while (x >= prim->get_width()) {
    x -= prim->get_width();
    y++;
  }
  if (prim->get_flags() & PRIM_FLAG_H_SCROLL) {
    prim->set_pixel_hscroll(bitmap, x, y, color);
  } else {
    prim->set_pixel(bitmap, x, y, color);
  }
}
