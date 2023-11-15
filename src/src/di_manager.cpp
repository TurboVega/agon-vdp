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

#include "di_general_line.h"
#include "di_horiz_line.h"
#include "di_set_pixel.h"
#include "di_vert_line.h"
#include "di_ellipse.h"
#include "di_solid_ellipse.h"
#include "di_rectangle.h"
#include "di_solid_rectangle.h"
#include "di_tile_map.h"
#include "di_bitmap.h"

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
extern bool cursorEnabled;
extern int videoMode;

extern "C" {
extern void fcn_copy_words_in_loop(void* dst, void* src, uint32_t num_words);
}

typedef enum {
  WritingActiveLines,
  ProcessingIncomingData,
  NearNewFrameStart
} LoopState;

// Default callback functions
void default_on_vertical_blank() {}

DiManager::DiManager() {
  m_next_buffer_write = 0;
  m_next_buffer_read = 0;
  m_num_buffer_chars = 0;
  m_terminal = NULL;
  m_cursor = NULL;
  m_flash_count = 0;
  m_on_vertical_blank_cb = &default_on_vertical_blank;
  memset(m_primitives, 0, sizeof(m_primitives));

  logicalCoords = false; // this mode always uses regular coordinates
}

DiManager::~DiManager() {
    clear();
}

void DiManager::create_root() {
  // The root primitive covers the entire screen, and is not drawn.
  // The application should define what the base layer of the screen
  // is (e.g., solid rectangle, terminal, tile map, etc.).

  DiPrimitive* root = new DiPrimitive;
  root->init_root();
  m_primitives[ROOT_PRIMITIVE_ID] = root;
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
      remove_primitive(old_prim);
    }

    parent->attach_child(prim);
    while (parent != m_primitives[ROOT_PRIMITIVE_ID] && !(parent->get_flags() & PRIM_FLAG_CLIP_KIDS)) {
      parent = parent->get_parent();
    }

    m_primitives[prim->get_id()] = prim;
    recompute_primitive(prim, 0, -1, -1);

    /*debug_log("\n-- Groups\n");
    for (int i = 0; i < 600; i++) {
      auto g = &m_groups[i];
      for (auto p = g->begin(); p != g->end(); p++) {
        if ((*p)->get_id() > 2) {
          debug_log("[%i] %u\n", i, (*p)->get_id());
        }
      }
    }*/
}

void DiManager::remove_primitive(DiPrimitive* prim) {
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
      remove_primitive(child);
      child = next;
    }

    m_primitives[prim->get_id()] = NULL;
    delete prim;
  }
}

void DiManager::recompute_primitive(DiPrimitive* prim, uint16_t old_flags,
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

      if (new_min_group < old_min_group) {
        // Add primitive to new groups that are above old groups
        int32_t end = MIN((new_max_group+1), old_min_group);
        for (int32_t g = new_min_group; g < end; g++) {
          std::vector<DiPrimitive*> * vp = &m_groups[g];
          vp->push_back(prim);
        }
      }

      if (new_max_group > old_max_group) {
        // Add primitive to new groups that are below old groups
        int32_t begin = MAX((old_max_group+1), new_min_group);
        for (int32_t g = begin; g <= new_max_group; g++) {
          std::vector<DiPrimitive*> * vp = &m_groups[g];
          vp->push_back(prim);
        }
      }

      prim->add_flags(PRIM_FLAGS_CAN_DRAW);
      //prim->generate_instructions();
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
      //prim->delete_instructions();
    }
  } else {
    if (new_use_groups) {
      // Just place primitive into new groups
      for (int32_t g = new_min_group; g <= new_max_group; g++) {
        std::vector<DiPrimitive*> * vp = &m_groups[g];
        vp->push_back(prim);
      }
      prim->add_flags(PRIM_FLAGS_CAN_DRAW);
      //prim->generate_instructions();
    } else {
      prim->remove_flags(PRIM_FLAGS_CAN_DRAW);
      //prim->delete_instructions();
    }
  }
}

DiPrimitive* DiManager::finish_create(uint16_t id, uint16_t flags, DiPrimitive* prim, DiPrimitive* parent_prim) {
    prim->set_id(id);
    prim->set_flags(flags);
    add_primitive(prim, parent_prim);
    return prim;
}

DiPrimitive* DiManager::create_point(uint16_t id, uint16_t parent, uint16_t flags,
                            int32_t x, int32_t y, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    DiPrimitive* prim = new DiSetPixel(x, y, color);

    return finish_create(id, flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_line(uint16_t id, uint16_t parent, uint16_t flags,
                            int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                            uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;
    auto sep_color = color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(sep_color);
    DiPrimitive* prim;
    if (x1 == x2) {
        if (y1 == y2) {
            prim = new DiSetPixel(x1, y1, color);
        } else if (y1 < y2) {
            auto line = new DiVerticalLine();
            line->make_line(flags, x1, y1, y2 - y1 + 1, color);
            prim = line;
        } else {
            auto line = new DiVerticalLine();
            line->make_line(flags, x1, y2, y1 - y2 + 1, color);
            prim = line;
        }
    } else if (x1 < x2) {
        if (y1 == y2) {
            auto line = new DiHorizontalLine();
            line->make_line(flags, x1, y1, x2 - x1 + 1, color);
            prim = line;
        } else if (y1 < y2) {
            if (y2 - y1 == x2 - x1) {
                auto line = new DiGeneralLine();
                line->make_line(flags, x1, y1, x2, y2, sep_color, opaqueness);
                prim = line;
            } else {
                auto line = new DiGeneralLine();
                line->make_line(flags, x1, y1, x2, y2, sep_color, opaqueness);
                prim = line;
            }
        } else {
            if (y2 - y1 == x2 - x1) {
                auto line = new DiGeneralLine();
                line->make_line(flags, x1, y1, x2, y2, sep_color, opaqueness);
                prim = line;
            } else {
                auto line = new DiGeneralLine();
                line->make_line(flags, x1, y1, x2, y2, sep_color, opaqueness);
                prim = line;
            }
        }
    } else {
        if (y1 == y2) {
            auto line = new DiHorizontalLine();
            line->make_line(flags, x2, y1, x1 - x2 + 1, color);
            prim = line;
        } else if (y1 < y2) {
            if (y2 - y1 == x1 - x2) {
                auto line = new DiGeneralLine();
                line->make_line(flags, x1, y1, x2, y2, sep_color, opaqueness);
                prim = line;
            } else {
                auto line = new DiGeneralLine();
                line->make_line(flags, x1, y1, x2, y2, sep_color, opaqueness);
                prim = line;
            }
        } else {
            if (y2 - y1 == x1 - x2) {
                auto line = new DiGeneralLine();
                line->make_line(flags, x1, y1, x2, y2, sep_color, opaqueness);
                prim = line;
            } else {
                auto line = new DiGeneralLine();
                line->make_line(flags, x1, y1, x2, y2, sep_color, opaqueness);
                prim = line;
            }
        }
    }

    return finish_create(id, flags, prim, parent_prim);
}

DiSolidRectangle* DiManager::create_solid_rectangle(OtfCmd_41_Create_primitive_Solid_Rectangle* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiSolidRectangle();
    prim->make_rectangle(cmd->m_flags, cmd->m_x, cmd->m_y, cmd->m_w, cmd->m_h, cmd->m_color);

    finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
    return prim;
}

DiPrimitive* DiManager::create_triangle_outline(OtfCmd_30_Create_primitive_Triangle_Outline* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_triangle_outline(cmd->m_flags, &cmd->m_x1, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_solid_triangle(OtfCmd_31_Create_primitive_Solid_Triangle* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_solid_triangle(cmd->m_flags, &cmd->m_x1, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_triangle_list_outline(OtfCmd_32_Create_primitive_Triangle_List_Outline* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_triangle_list_outline(cmd->m_flags, cmd->m_coords, cmd->m_n, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_solid_triangle_list(OtfCmd_33_Create_primitive_Solid_Triangle_List* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_solid_triangle_list(cmd->m_flags, cmd->m_coords, cmd->m_n, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_triangle_fan_outline(OtfCmd_34_Create_primitive_Triangle_Fan_Outline* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_triangle_fan_outline(cmd->m_flags, &cmd->m_sx0, cmd->m_n, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_solid_triangle_fan(OtfCmd_35_Create_primitive_Solid_Triangle_Fan* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_solid_triangle_fan(cmd->m_flags, &cmd->m_sx0, cmd->m_n, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_triangle_strip_outline(OtfCmd_36_Create_primitive_Triangle_Strip_Outline* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_triangle_strip_outline(cmd->m_flags, &cmd->m_sx0, cmd->m_n, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_solid_triangle_strip(OtfCmd_37_Create_primitive_Solid_Triangle_Strip* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_solid_triangle_strip(cmd->m_flags, &cmd->m_sx0, cmd->m_n, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_quad_outline(OtfCmd_60_Create_primitive_Quad_Outline* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_quad_outline(cmd->m_flags, &cmd->m_x1, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_solid_quad(OtfCmd_61_Create_primitive_Solid_Quad* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_solid_quad(cmd->m_flags, &cmd->m_x1, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_quad_list_outline(OtfCmd_62_Create_primitive_Quad_List_Outline* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_quad_list_outline(cmd->m_flags, cmd->m_coords, cmd->m_n, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_solid_quad_list(OtfCmd_63_Create_primitive_Solid_Quad_List* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_solid_quad_list(cmd->m_flags, cmd->m_coords, cmd->m_n, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_quad_strip_outline(OtfCmd_64_Create_primitive_Quad_Strip_Outline* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_quad_strip_outline(cmd->m_flags, &cmd->m_sx0, cmd->m_n, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_solid_quad_strip(OtfCmd_65_Create_primitive_Solid_Quad_Strip* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiGeneralLine();
    auto color = cmd->m_color;
    uint8_t opaqueness = DiPrimitive::normal_alpha_to_opaqueness(color);
    prim->make_solid_quad_strip(cmd->m_flags, &cmd->m_sx0, cmd->m_n, color, opaqueness);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiTileMap* DiManager::create_tile_map(uint16_t id, uint16_t parent, uint16_t flags,
                            int32_t screen_width, int32_t screen_height,
                            uint32_t columns, uint32_t rows,
                            uint32_t width, uint32_t height) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    flags |= PRIM_FLAGS_X_SRC;
    DiTileMap* tile_map =
      new DiTileMap(screen_width, screen_height, columns, rows, width, height, flags);

    finish_create(id, flags, tile_map, parent_prim);
    return tile_map;
}

DiTileArray* DiManager::create_tile_array(uint16_t id, uint16_t parent, uint16_t flags,
                            int32_t screen_width, int32_t screen_height,
                            uint32_t columns, uint32_t rows,
                            uint32_t width, uint32_t height) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    flags |= PRIM_FLAGS_X_SRC;
    DiTileArray* tile_array =
      new DiTileArray(screen_width, screen_height, columns, rows, width, height, flags);

    finish_create(id, flags, tile_array, parent_prim);
    return tile_array;
}

DiTerminal* DiManager::create_terminal(uint16_t id, uint16_t parent, uint16_t flags,
                            uint32_t x, uint32_t y, uint32_t columns, uint32_t rows, const uint8_t* font) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    flags |= PRIM_FLAGS_X_SRC|PRIM_FLAGS_ALL_SAME;
    DiTerminal* terminal = new DiTerminal(x, y, flags, columns, rows, font);

    finish_create(id, flags, terminal, parent_prim);
    m_terminal = terminal;

    // Create a child rectangle as a text cursor.
    int16_t cx, cy, cx_extent, cy_extent;
    cx = cy = cx_extent = cy_extent = 0;
    m_terminal->get_tile_coordinates(0, 0, cx, cy, cx_extent, cy_extent);
    auto w = cx_extent - cx;

    OtfCmd_41_Create_primitive_Solid_Rectangle cmd;
    cmd.m_id = id + 1;
    cmd.m_pid = id;
    cmd.m_flags = flags & PRIM_FLAGS_DEFAULT;
    cmd.m_x = cx;
    cmd.m_y = cy_extent - 2;
    cmd.m_w = w;
    cmd.m_h = 2;
    cmd.m_color = 0xFF;
    m_cursor = create_solid_rectangle(&cmd);
    cursorEnabled = true;
    terminalMode = true;

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
    } else if (loop_state == LoopState::WritingActiveLines) {
      // Timing just moved into the vertical blanking area.
      process_stored_characters();
      while (ESPSerial.available() > 0) {
        process_character(ESPSerial.read());
      }
      (*m_on_vertical_blank_cb)();

      if (terminalMode && cursorEnabled && m_cursor) {
        auto flags = m_cursor->get_flags();
        auto cid = m_cursor->get_id();
        if ((flags & PRIM_FLAG_PAINT_THIS) == 0) {
          if (++m_flash_count >= 50) {
            // turn ON cursor
            m_terminal->bring_current_position_into_view();
            int16_t cx, cy, cx_extent, cy_extent;
            cx = cy = cx_extent = cy_extent = 0;
            uint16_t col = 0;
            uint16_t row = 0;
            m_terminal->get_position(col, row);
            m_terminal->get_tile_coordinates(col, row, cx, cy, cx_extent, cy_extent);
            auto w = cx_extent - cx;
            set_primitive_flags(cid, flags | PRIM_FLAG_PAINT_THIS);
            move_primitive_absolute(cid, cx, cy_extent-2);
            m_flash_count = 0;
          }
        } else {
          if (++m_flash_count >= 10) {
            // turn OFF cursor
            set_primitive_flags(cid, flags ^ PRIM_FLAG_PAINT_THIS);
            m_flash_count = 0;
          }
        }
      }

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
  if (m_incoming_command.size()) {
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
  m_incoming_command.push_back(character);
  if (m_incoming_command.size() >= len) {
    m_incoming_command.clear();
    return true;
  }
  return false;
}

bool DiManager::define_graphics_viewport(uint8_t character) {
  m_incoming_command.push_back(character);
  if (m_incoming_command.size() >= 9) {
      int16_t left = get_param_16(1);
      int16_t bottom = get_param_16(3);
      int16_t right = get_param_16(5);
      int16_t top = get_param_16(7);
      m_incoming_command.clear();
      return true;
  }
  return false;
}

bool DiManager::define_text_viewport(uint8_t character) {
  m_incoming_command.push_back(character);
  if (m_incoming_command.size() >= 5) {
      uint8_t left = get_param_8(1);
      uint8_t bottom = get_param_8(2);
      uint8_t right = get_param_8(3);
      uint8_t top = get_param_8(4);
      m_incoming_command.clear();
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

From this page: https://www.bbcbasic.co.uk/bbcwin/manual/bbcwin8.html#vdu23
VDU 23, 1, 0; 0; 0; 0;: Text Cursor Control
*/
bool DiManager::handle_udg_sys_cmd(uint8_t character) {
  m_incoming_command.push_back(character);
  if (m_incoming_command.size() >= 2 && get_param_8(1) == 30) {
    return handle_otf_cmd();
  }
  if (m_incoming_command.size() >= 2 && get_param_8(1) == 1) {
    // VDU 23, 1, enable; 0; 0; 0;: Text Cursor Control
    if (m_incoming_command.size() >= 10) {
      if (m_terminal) {
        auto flags = m_cursor->get_flags();
        if (get_param_8(2) != 0 && cursorEnabled && (flags & PRIM_FLAG_PAINT_THIS == 0)) {
          // turn ON cursor
          set_primitive_flags(m_cursor->get_id(), flags | PRIM_FLAG_PAINT_THIS);
        } else {
          if (flags & PRIM_FLAG_PAINT_THIS != 0) {
            // turn OFF cursor
            set_primitive_flags(m_cursor->get_id(), flags ^ PRIM_FLAG_PAINT_THIS);
          }
        }
      }
      return true;
    }
    return false;
  }
  if (m_incoming_command.size() >= 3) {
    switch (m_incoming_command[2]) {

      // VDU 23, 0, &80, b: General poll
      case VDP_GP: /*0x80*/ {
        if (m_incoming_command.size() == 4) {
          uint8_t echo = get_param_8(3);
          send_general_poll(echo);
          m_incoming_command.clear();
          return true;
        }
      } break;

      // VDU 23, 0, &81, n: Set the keyboard locale (0=UK, 1=US, etc)
      case VDP_KEYCODE: /*0x81*/ {
        if (m_incoming_command.size() == 4) {
          uint8_t region = get_param_8(3);
          vdu_sys_video_kblayout(region);
          m_incoming_command.clear();
          return true;
        }
      } break;

      // VDU 23, 0, &82: Request cursor position
      case VDP_CURSOR: /*0x82*/ {
        if (m_incoming_command.size() == 3) {
          send_cursor_position();
          m_incoming_command.clear();
          return true;
        }
      } break;

      // VDU 23, 0, &83, x; y;: Get ASCII code of character at character position x, y
      case VDP_SCRCHAR: /*0x83*/ {
        if (m_incoming_command.size() == 7) {
          int32_t x = get_param_16(3);
          int32_t y = get_param_16(5);
          send_screen_char(x, y);
          m_incoming_command.clear();
          return true;
        }
      } break;

      // VDU 23, 0, &84, x; y;: Get colour of pixel at pixel position x, y
      case VDP_SCRPIXEL: /*0x84*/ {
        if (m_incoming_command.size() == 7) {
          int32_t x = get_param_16(3);
          int32_t y = get_param_16(5);
          send_screen_pixel(x, y);
          m_incoming_command.clear();
          return true;
        }
      } break;

      // VDU 23, 0, &85, channel, waveform, volume, freq; duration;: Send a note to the VDP audio driver
      case VDP_AUDIO: /*0x85*/ {
        if (m_incoming_command.size() == 10) {
          m_incoming_command.clear();
          return true;
        }
      } break;

      // VDU 23, 0, &86: Fetch the screen dimensions
      case VDP_MODE: /*0x86*/ {
        if (m_incoming_command.size() == 3) {
          send_mode_information();
          m_incoming_command.clear();
          return true;
        }
      } break;

      // VDU 23, 0, &87: RTC control (Requires MOS 1.03 or above)
      case VDP_RTC: /*0x87*/ {
        if (m_incoming_command.size() == 3) {
          m_incoming_command.clear();
          return true;
        }
      } break;

      // VDU 23, 0, &88, delay; rate; led: Keyboard Control (Requires MOS 1.03 or above)
      case VDP_KEYSTATE: /*0x88*/ {
        if (m_incoming_command.size() == 8) {
          sendKeyboardState();
          m_incoming_command.clear();
          return true;
        }
      } break;

      // VDU 23, 0, &C0, n: Turn logical screen scaling on and off, where 1=on and 0=off (Requires MOS 1.03 or above)
      case VDP_LOGICALCOORDS: /*0xC0*/ {
        if (m_incoming_command.size() == 4) {
          // This command is ignored; this mode always uses regular coordinates.
          m_incoming_command.clear();
          return true;
        }
      } break;

      // VDU 23, 0, &FF: Switch to terminal mode for CP/M (This will disable keyboard entry in BBC BASIC/MOS)
      case VDP_TERMINALMODE: /*0xFF*/ {
        if (m_incoming_command.size() == 3) {
          // This command is ignored; this mode is terminal mode.
          m_incoming_command.clear();
          return true;
        }
      } break;

      default: {
        m_incoming_command.clear();
        return true;
      }
    }
  }
  return false;
}

// Process 800x600x64 On-the-Fly Command Set
//
bool DiManager::handle_otf_cmd() {
  if (m_incoming_command.size() >= 5) {
    OtfCmdUnion* cu = (OtfCmdUnion*)(&m_incoming_command[0]);
    switch (m_incoming_command[2]) {

      case 0: {
        auto cmd = &cu->m_0_Set_flags_for_primitive;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_primitive_flags(cmd->m_id, cmd->m_flags);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 1: {
        auto cmd = &cu->m_1_Set_primitive_position;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          move_primitive_absolute(cmd->m_id, cmd->m_x, cmd->m_y);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 2: {
        auto cmd = &cu->m_2_Adjust_primitive_position;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          move_primitive_relative(cmd->m_id, cmd->m_ix, cmd->m_iy);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 3: {
        auto cmd = &cu->m_3_Delete_primitive;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          delete_primitive(cmd->m_id);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 4: {
        auto cmd = &cu->m_4_Generate_code_for_primitive;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          generate_code_for_primitive(cmd->m_id);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 10: {
        auto cmd = &cu->m_10_Create_primitive_Point;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_point(cmd->m_id, cmd->m_pid, cmd->m_flags, cmd->m_x, cmd->m_y, cmd->m_color);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 20: {
        auto cmd = &cu->m_20_Create_primitive_Line;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_line(cmd->m_id, cmd->m_pid, cmd->m_flags,
            cmd->m_x1, cmd->m_y1, cmd->m_x2, cmd->m_y2, cmd->m_color);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 30: {
        auto cmd = &cu->m_30_Create_primitive_Triangle_Outline;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_triangle_outline(cmd);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 31: {
        auto cmd = &cu->m_31_Create_primitive_Solid_Triangle;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_solid_triangle(cmd);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 32: {
        auto cmd = &cu->m_32_Create_primitive_Triangle_List_Outline;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          auto total_size = sizeof(*cmd) - sizeof(cmd->m_coords) + ((uint32_t)cmd->m_n * 3 * sizeof(uint16_t));
          if (len >= total_size) {
            create_triangle_list_outline(cmd);
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 33: {
        auto cmd = &cu->m_33_Create_primitive_Solid_Triangle_List;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          auto total_size = sizeof(*cmd) - sizeof(cmd->m_coords) + ((uint32_t)cmd->m_n * 3 * sizeof(uint16_t));
          if (len >= total_size) {
            create_solid_triangle_list(cmd);
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 34: {
        auto cmd = &cu->m_34_Create_primitive_Triangle_Fan_Outline;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          auto total_size = sizeof(*cmd) - sizeof(cmd->m_coords) + (((uint32_t)cmd->m_n + 2) * sizeof(uint16_t));
          if (len >= total_size) {
            create_triangle_fan_outline(cmd);
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 35: {
        auto cmd = &cu->m_35_Create_primitive_Solid_Triangle_Fan;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          auto total_size = sizeof(*cmd) - sizeof(cmd->m_coords) + (((uint32_t)cmd->m_n + 2) * sizeof(uint16_t));
          if (len >= total_size) {
            create_solid_triangle_fan(cmd);
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 36: {
        auto cmd = &cu->m_36_Create_primitive_Triangle_Strip_Outline;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          auto total_size = sizeof(*cmd) - sizeof(cmd->m_coords) + (((uint32_t)cmd->m_n + 2) * sizeof(uint16_t));
          if (len >= total_size) {
            create_triangle_strip_outline(cmd);
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 37: {
        auto cmd = &cu->m_37_Create_primitive_Solid_Triangle_Strip;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          auto total_size = sizeof(*cmd) - sizeof(cmd->m_coords) + (((uint32_t)cmd->m_n + 2) * sizeof(uint16_t));
          if (len >= total_size) {
            create_solid_triangle_strip(cmd);
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 40: {
        auto cmd = &cu->m_40_Create_primitive_Rectangle_Outline;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_rectangle_outline(cmd);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 41: {
        auto cmd = &cu->m_41_Create_primitive_Solid_Rectangle;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_solid_rectangle(cmd);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 50: {
        auto cmd = &cu->m_50_Create_primitive_Ellipse_Outline;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_ellipse(cmd->m_id, cmd->m_pid, cmd->m_flags, cmd->m_x,
            cmd->m_y, cmd->m_w, cmd->m_h, cmd->m_color);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 51: {
        auto cmd = &cu->m_51_Create_primitive_Solid_Ellipse;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_solid_ellipse(cmd->m_id, cmd->m_pid, cmd->m_flags,
            cmd->m_x, cmd->m_y, cmd->m_w, cmd->m_h, cmd->m_color);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 60: {
        auto cmd = &cu->m_60_Create_primitive_Quad_Outline;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_quad_outline(cmd);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 61: {
        auto cmd = &cu->m_61_Create_primitive_Solid_Quad;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_solid_quad(cmd);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 62: {
        auto cmd = &cu->m_62_Create_primitive_Quad_List_Outline;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          auto total_size = sizeof(*cmd) - sizeof(cmd->m_coords) + ((uint32_t)cmd->m_n * 4 * sizeof(uint16_t));
          if (len >= total_size) {
            create_quad_list_outline(cmd);
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 63: {
        auto cmd = &cu->m_63_Create_primitive_Solid_Quad_List;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          auto total_size = sizeof(*cmd) - sizeof(cmd->m_coords) + ((uint32_t)cmd->m_n * 4 * sizeof(uint16_t));
          if (len >= total_size) {
            create_solid_quad_list(cmd);
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 64: {
        auto cmd = &cu->m_64_Create_primitive_Quad_Strip_Outline;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          auto total_size = sizeof(*cmd) - sizeof(cmd->m_coords) + (((uint32_t)cmd->m_n * 2 + 2) * sizeof(uint16_t));
          if (len >= total_size) {
            create_quad_strip_outline(cmd);
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 65: {
        auto cmd = &cu->m_65_Create_primitive_Solid_Quad_Strip;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          auto total_size = sizeof(*cmd) - sizeof(cmd->m_coords) + (((uint32_t)cmd->m_n * 2 + 2) * sizeof(uint16_t));
          if (len >= total_size) {
            create_solid_quad_strip(cmd);
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 80: {
        auto cmd = &cu->m_80_Create_primitive_Tile_Array;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_tile_array(cmd->m_id, cmd->m_pid, cmd->m_flags,
            ACT_PIXELS, ACT_LINES,
            cmd->m_columns, cmd->m_rows, cmd->m_w, cmd->m_h);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 81: {
        auto cmd = &cu->m_81_Create_Solid_Bitmap_for_Tile_Array;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_solid_bitmap_for_tile_array(cmd->m_id, cmd->m_bmid);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 82: {
        auto cmd = &cu->m_82_Create_Masked_Bitmap_for_Tile_Array;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_masked_bitmap_for_tile_array(cmd->m_id, cmd->m_bmid, cmd->m_color);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 83: {
        auto cmd = &cu->m_83_Create_Transparent_Bitmap_for_Tile_Array;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_transparent_bitmap_for_tile_array(cmd->m_id, cmd->m_bmid, cmd->m_color);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 84: {
        auto cmd = &cu->m_84_Set_bitmap_ID_for_tile_in_Tile_Array;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_tile_array_bitmap_id(cmd->m_id, cmd->m_column, cmd->m_row, cmd->m_bmid);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 85: {
        auto cmd = &cu->m_85_Set_solid_bitmap_pixel_in_Tile_Array;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_solid_bitmap_pixel_for_tile_array(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_color, 0);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 86: {
        auto cmd = &cu->m_86_Set_masked_bitmap_pixel_in_Tile_Array;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_masked_bitmap_pixel_for_tile_array(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_color, 0);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 87: {
        auto cmd = &cu->m_87_Set_transparent_bitmap_pixel_in_Tile_Array;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_transparent_bitmap_pixel_for_tile_array(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_color, 0);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 88: {
        auto cmd = &cu->m_88_Set_solid_bitmap_pixels_in_Tile_Array;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          set_solid_bitmap_pixel_for_tile_array(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_colors[len-sizeof(*cmd)], m_command_data_index);
          if (++m_command_data_index >= cmd->m_n) {
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 89: {
        auto cmd = &cu->m_89_Set_masked_bitmap_pixels_in_Tile_Array;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          set_masked_bitmap_pixel_for_tile_array(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_colors[len-sizeof(*cmd)], m_command_data_index);
          if (++m_command_data_index >= cmd->m_n) {
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 90: {
        auto cmd = &cu->m_90_Set_transparent_bitmap_pixels_in_Tile_Array;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          set_transparent_bitmap_pixel_for_tile_array(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_colors[len-sizeof(*cmd)], m_command_data_index);
          if (++m_command_data_index >= cmd->m_n) {
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 100: {
        auto cmd = &cu->m_100_Create_primitive_Tile_Map;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_tile_map(cmd->m_id, cmd->m_pid, cmd->m_flags, ACT_PIXELS, ACT_LINES,
            cmd->m_columns, cmd->m_rows, cmd->m_w, cmd->m_h);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 101: {
        auto cmd = &cu->m_101_Create_Solid_Bitmap_for_Tile_Map;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_solid_bitmap_for_tile_map(cmd->m_id, cmd->m_bmid);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 102: {
        auto cmd = &cu->m_102_Create_Masked_Bitmap_for_Tile_Map;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_masked_bitmap_for_tile_map(cmd->m_id, cmd->m_bmid, cmd->m_color);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 103: {
        auto cmd = &cu->m_103_Create_Transparent_Bitmap_for_Tile_Map;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_transparent_bitmap_for_tile_map(cmd->m_id, cmd->m_bmid, cmd->m_color);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 104: {
        auto cmd = &cu->m_104_Set_bitmap_ID_for_tile_in_Tile_Map;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_tile_map_bitmap_id(cmd->m_id, cmd->m_column, cmd->m_row, cmd->m_bmid);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 105: {
        auto cmd = &cu->m_105_Set_solid_bitmap_pixel_in_Tile_Map;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_solid_bitmap_pixel_for_tile_map(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_color, 0);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 106: {
        auto cmd = &cu->m_106_Set_masked_bitmap_pixel_in_Tile_Map;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_masked_bitmap_pixel_for_tile_map(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_color, 0);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 107: {
        auto cmd = &cu->m_107_Set_transparent_bitmap_pixel_in_Tile_Map;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_transparent_bitmap_pixel_for_tile_map(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_color, 0);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 108: {
        auto cmd = &cu->m_108_Set_solid_bitmap_pixels_in_Tile_Map;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          set_solid_bitmap_pixel_for_tile_map(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_colors[len-sizeof(*cmd)], m_command_data_index);
          if (++m_command_data_index >= cmd->m_n) {
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 109: {
        auto cmd = &cu->m_109_Set_masked_bitmap_pixels_in_Tile_Map;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          set_masked_bitmap_pixel_for_tile_map(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_colors[len-sizeof(*cmd)], m_command_data_index);
          if (++m_command_data_index >= cmd->m_n) {
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 110: {
        auto cmd = &cu->m_110_Set_transparent_bitmap_pixels_in_Tile_Map;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          set_transparent_bitmap_pixel_for_tile_map(cmd->m_id, cmd->m_bmid, cmd->m_x, cmd->m_y,
            cmd->m_colors[len-sizeof(*cmd)], m_command_data_index);
          if (++m_command_data_index >= cmd->m_n) {
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 120: {
        auto cmd = &cu->m_120_Create_primitive_Solid_Bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_solid_bitmap(cmd->m_id, cmd->m_pid, cmd->m_flags, cmd->m_w, cmd->m_h);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 121: {
        auto cmd = &cu->m_121_Create_primitive_Masked_Bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_masked_bitmap(cmd->m_id, cmd->m_pid, cmd->m_flags, cmd->m_w, cmd->m_h, cmd->m_color);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 122: {
        auto cmd = &cu->m_122_Create_primitive_Transparent_Bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_transparent_bitmap(cmd->m_id, cmd->m_pid, cmd->m_flags, cmd->m_w, cmd->m_h, cmd->m_color);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 123: {
        auto cmd = &cu->m_123_Set_position_and_slice_solid_bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          slice_solid_bitmap_absolute(cmd->m_id, cmd->m_x, cmd->m_y, cmd->m_s, cmd->m_h);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 124: {
        auto cmd = &cu->m_124_Set_position_and_slice_masked_bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          slice_masked_bitmap_absolute(cmd->m_id, cmd->m_x, cmd->m_y, cmd->m_s, cmd->m_h);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 125: {
        auto cmd = &cu->m_125_Set_position_and_slice_transparent_bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          slice_transparent_bitmap_absolute(cmd->m_id, cmd->m_x, cmd->m_y, cmd->m_s, cmd->m_h);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 126: {
        auto cmd = &cu->m_126_Adjust_position_and_slice_solid_bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          slice_solid_bitmap_relative(cmd->m_id, cmd->m_x, cmd->m_y, cmd->m_s, cmd->m_h);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 127: {
        auto cmd = &cu->m_127_Adjust_position_and_slice_masked_bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          slice_masked_bitmap_relative(cmd->m_id, cmd->m_x, cmd->m_y, cmd->m_s, cmd->m_h);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 128: {
        auto cmd = &cu->m_128_Adjust_position_and_slice_transparent_bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          slice_transparent_bitmap_relative(cmd->m_id, cmd->m_x, cmd->m_y, cmd->m_s, cmd->m_h);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 129: {
        auto cmd = &cu->m_129_Set_solid_bitmap_pixel;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_solid_bitmap_pixel(cmd->m_id, cmd->m_x, cmd->m_y, cmd->m_color, 0);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 130: {
        auto cmd = &cu->m_130_Set_masked_bitmap_pixel;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_masked_bitmap_pixel(cmd->m_id, cmd->m_x, cmd->m_y, cmd->m_color, 0);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 131: {
        auto cmd = &cu->m_131_Set_transparent_bitmap_pixel;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          set_transparent_bitmap_pixel(cmd->m_id, cmd->m_x, cmd->m_y, cmd->m_color, 0);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 132: {
        auto cmd = &cu->m_132_Set_solid_bitmap_pixels;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          set_solid_bitmap_pixel(cmd->m_id, cmd->m_x, cmd->m_y,
            cmd->m_colors[len-sizeof(*cmd)], m_command_data_index);
          if (++m_command_data_index >= cmd->m_n) {
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 133: {
        auto cmd = &cu->m_133_Set_masked_bitmap_pixels;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          set_masked_bitmap_pixel(cmd->m_id, cmd->m_x, cmd->m_y,
            cmd->m_colors[len-sizeof(*cmd)], m_command_data_index);
          if (++m_command_data_index >= cmd->m_n) {
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 134: {
        auto cmd = &cu->m_134_Set_transparent_bitmap_pixels;
        auto len = m_incoming_command.size();
        if (len >= sizeof(*cmd)) {
          set_transparent_bitmap_pixel(cmd->m_id, cmd->m_x, cmd->m_y,
            cmd->m_colors[len-sizeof(*cmd)], m_command_data_index);
          if (++m_command_data_index >= cmd->m_n) {
            m_incoming_command.clear();
            return true;
          }
        } else if (m_incoming_command.size() == 5) {
          m_command_data_index = 0;
        }
      } break;

      case 135: {
        auto cmd = &cu->m_135_Create_primitive_Reference_Solid_Bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_reference_solid_bitmap(cmd->m_id, cmd->m_pid, cmd->m_flags, cmd->m_bmid);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 136: {
        auto cmd = &cu->m_136_Create_primitive_Reference_Masked_Bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_reference_masked_bitmap(cmd->m_id, cmd->m_pid, cmd->m_flags, cmd->m_bmid);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 137: {
        auto cmd = &cu->m_137_Create_primitive_Reference_Transparent_Bitmap;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_reference_transparent_bitmap(cmd->m_id, cmd->m_pid, cmd->m_flags, cmd->m_bmid);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 140: {
        auto cmd = &cu->m_140_Create_primitive_Group;
        if (m_incoming_command.size() == sizeof(*cmd)) {
          create_primitive_group(cmd->m_id, cmd->m_pid, cmd->m_flags, cmd->m_x, cmd->m_y, cmd->m_w, cmd->m_h);
          m_incoming_command.clear();
          return true;
        }
      } break;

      case 150: {
        auto cmd = &cu->m_150_Create_primitive_Terminal;
      } break;

      case 151: {
        auto cmd = &cu->m_151_Select_Active_Terminal;
      } break;

      case 152: {
        auto cmd = &cu->m_152_Define_Terminal_Character;
      } break;

      case 153: {
        auto cmd = &cu->m_153_Define_Terminal_Character_Range;
      } break;

      case 200: {
        auto cmd = &cu->m_200_Create_primitive_Render_3D_Scene;
      } break;

      case 201: {
        auto cmd = &cu->m_201_Define_Mesh_Vertices;
      } break;

      case 202: {
        auto cmd = &cu->m_202_Set_Mesh_Vertex_Indices;
      } break;

      case 203: {
        auto cmd = &cu->m_203_Define_Texture_Coordinates;
      } break;

      case 204: {
        auto cmd = &cu->m_204_Set_Texture_Coordinate_Indices;
      } break;

      case 205: {
        auto cmd = &cu->m_205_Create_Object;
      } break;

      case 206: {
        auto cmd = &cu->m_206_Set_Object_X_Scale_Factor;
      } break;

      case 207: {
        auto cmd = &cu->m_207_Set_Object_Y_Scale_Factor;
      } break;

      case 208: {
        auto cmd = &cu->m_208_Set_Object_Z_Scale_Factor;
      } break;

      case 209: {
        auto cmd = &cu->m_209_Set_Object_XYZ_Scale_Factors;
      } break;

      case 210: {
        auto cmd = &cu->m_210_Set_Object_X_Rotation_Angle;
      } break;

      case 211: {
        auto cmd = &cu->m_211_Set_Object_Y_Rotation_Angle;
      } break;

      case 212: {
        auto cmd = &cu->m_212_Set_Object_Z_Rotation_Angle;
      } break;

      case 213: {
        auto cmd = &cu->m_213_Set_Object_XYZ_Rotation_Angles;
      } break;

      case 214: {
        auto cmd = &cu->m_214_Set_Object_X_Translation_Distance;
      } break;

      case 215: {
        auto cmd = &cu->m_215_Set_Object_Y_Translation_Distance;
      } break;

      case 216: {
        auto cmd = &cu->m_216_Set_Object_Z_Translation_Distance;
      } break;

      case 217: {
        auto cmd = &cu->m_217_Set_Object_XYZ_Translation_Distances;
      } break;

      case 218: {
        auto cmd = &cu->m_218_Render_To_Bitmap;
      } break;

      default: {
        m_incoming_command.clear();
        return true; // ignore the command
      }
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
  m_incoming_command.push_back(character);
  if (m_incoming_command.size() >= 3) {
    if (m_terminal) {
      uint8_t x = get_param_8(1);
      uint8_t y = get_param_8(2);
      m_terminal->move_cursor_tab(x, y);
    }
    m_incoming_command.clear();
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

void DiManager::set_primitive_flags(uint16_t id, uint16_t flags) {
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
  remove_primitive(prim);  
}

void DiManager::generate_code_for_primitive(uint16_t id) {
  DiPrimitive* prim; if (!(prim = (DiPrimitive*)get_safe_primitive(id))) return;
  prim->delete_instructions();
  prim->generate_instructions();
}

DiPrimitive* DiManager::create_rectangle_outline(OtfCmd_40_Create_primitive_Rectangle_Outline* cmd) {
    if (!validate_id(cmd->m_id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(cmd->m_pid))) return NULL;

    auto prim = new DiRectangle();
    debug_log("%hX %hi %hi %hu %hu %02hX\n",cmd->m_flags, cmd->m_x, cmd->m_y, cmd->m_w, cmd->m_h, cmd->m_color);
    prim->make_rectangle_outline(cmd->m_flags, cmd->m_x, cmd->m_y, cmd->m_w, cmd->m_h, cmd->m_color);

    return finish_create(cmd->m_id, cmd->m_flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_ellipse(uint16_t id, uint16_t parent, uint16_t flags,
                        int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiEllipse();
    prim->init_params(flags, x, y, width, height, color);

    return finish_create(id, flags, prim, parent_prim);
}

DiPrimitive* DiManager::create_solid_ellipse(uint16_t id, uint16_t parent, uint16_t flags,
                        int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiSolidEllipse();
    prim->init_params(flags, x, y, width, height, color);

    return finish_create(id, flags, prim, parent_prim);
}

DiBitmap* DiManager::create_solid_bitmap(uint16_t id, uint16_t parent, uint16_t flags,
                        uint32_t width, uint32_t height) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiBitmap(width, height, flags);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiBitmap* DiManager::create_masked_bitmap(uint16_t id, uint16_t parent, uint16_t flags,
                        uint32_t width, uint32_t height, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiBitmap(width, height, flags);
    prim->set_transparent_color(color);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiBitmap* DiManager::create_transparent_bitmap(uint16_t id, uint16_t parent, uint16_t flags,
                        uint32_t width, uint32_t height, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiBitmap(width, height, flags);
    prim->set_transparent_color(color);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiBitmap* DiManager::create_reference_solid_bitmap(uint16_t id, uint16_t parent, uint16_t flags, uint16_t bmid) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;
    DiBitmap* ref_prim; if (!(ref_prim = (DiBitmap*) get_safe_primitive(bmid))) return NULL;

    auto prim = new DiBitmap(flags, ref_prim);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiBitmap* DiManager::create_reference_masked_bitmap(uint16_t id, uint16_t parent, uint16_t flags, uint16_t bmid) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;
    DiBitmap* ref_prim; if (!(ref_prim = (DiBitmap*) get_safe_primitive(bmid))) return NULL;

    auto prim = new DiBitmap(flags, ref_prim);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiBitmap* DiManager::create_reference_transparent_bitmap(uint16_t id, uint16_t parent, uint16_t flags, uint16_t bmid) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;
    DiBitmap* ref_prim; if (!(ref_prim = (DiBitmap*) get_safe_primitive(bmid))) return NULL;

    auto prim = new DiBitmap(flags, ref_prim);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiTileBitmap* DiManager::create_solid_bitmap_for_tile_array(uint16_t id, uint16_t bm_id) {
    DiTileArray* prim; if (!(prim = (DiTileArray*)get_safe_primitive(id))) return NULL;
    auto bitmap = prim->create_bitmap(bm_id);
    return bitmap;
}

DiTileBitmap* DiManager::create_masked_bitmap_for_tile_array(uint16_t id, uint16_t bm_id, uint8_t color) {
    DiTileArray* prim; if (!(prim = (DiTileArray*)get_safe_primitive(id))) return NULL;
    auto bitmap = prim->create_bitmap(bm_id);
    bitmap->set_transparent_color(color);
    return bitmap;
}

DiTileBitmap* DiManager::create_transparent_bitmap_for_tile_array(uint16_t id, uint16_t bm_id, uint8_t color) {
    DiTileArray* prim; if (!(prim = (DiTileArray*)get_safe_primitive(id))) return NULL;
    auto bitmap = prim->create_bitmap(bm_id);
    bitmap->set_transparent_color(color);
    return bitmap;
}

DiTileBitmap* DiManager::create_solid_bitmap_for_tile_map(uint16_t id, uint16_t bm_id) {
    DiTileMap* prim; if (!(prim = (DiTileMap*)get_safe_primitive(id))) return NULL;
    auto bitmap = prim->create_bitmap(bm_id);
    return bitmap;
}

DiTileBitmap* DiManager::create_masked_bitmap_for_tile_map(uint16_t id, uint16_t bm_id, uint8_t color) {
    DiTileMap* prim; if (!(prim = (DiTileMap*)get_safe_primitive(id))) return NULL;
    auto bitmap = prim->create_bitmap(bm_id);
    bitmap->set_transparent_color(color);
    return bitmap;
}

DiTileBitmap* DiManager::create_transparent_bitmap_for_tile_map(uint16_t id, uint16_t bm_id, uint8_t color) {
    DiTileMap* prim; if (!(prim = (DiTileMap*)get_safe_primitive(id))) return NULL;
    auto bitmap = prim->create_bitmap(bm_id);
    bitmap->set_transparent_color(color);
    return bitmap;
}

DiRender* DiManager::create_solid_render(uint16_t id, uint16_t parent, uint16_t flags,
                        uint32_t width, uint32_t height) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiRender(width, height, flags);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiRender* DiManager::create_masked_render(uint16_t id, uint16_t parent, uint16_t flags,
                        uint32_t width, uint32_t height) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiRender(width, height, flags);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiRender* DiManager::create_transparent_render(uint16_t id, uint16_t parent, uint16_t flags,
                        uint32_t width, uint32_t height, uint8_t color) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    auto prim = new DiRender(width, height, flags);
    prim->set_transparent_color(color);

    finish_create(id, flags, prim, parent_prim);
    return prim;
}

DiPrimitive* DiManager::create_primitive_group(uint16_t id, uint16_t parent, uint16_t flags,
                        int32_t x, int32_t y, uint32_t width, uint32_t height) {
    if (!validate_id(id)) return NULL;
    DiPrimitive* parent_prim; if (!(parent_prim = get_safe_primitive(parent))) return NULL;

    DiPrimitive* prim = new DiPrimitive();
    prim->set_relative_position(x, y);
    prim->set_size(width, height);

    return finish_create(id, flags, prim, parent_prim);
}

void DiManager::slice_solid_bitmap_absolute(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height) {
  DiBitmap* prim; if (!(prim = (DiBitmap*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  prim->set_slice_position(x, y, start_line, height);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group);
}

void DiManager::slice_masked_bitmap_absolute(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height) {
  DiBitmap* prim; if (!(prim = (DiBitmap*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  prim->set_slice_position(x, y, start_line, height);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group);
}

void DiManager::slice_transparent_bitmap_absolute(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height) {
  DiBitmap* prim; if (!(prim = (DiBitmap*)get_safe_primitive(id))) return;
  auto old_flags = prim->get_flags();
  int32_t old_min_group = -1, old_max_group = -1;
  if (old_flags & PRIM_FLAGS_CAN_DRAW) {
    prim->get_vertical_group_range(old_min_group, old_max_group);
  }
  prim->set_slice_position(x, y, start_line, height);
  recompute_primitive(prim, old_flags, old_min_group, old_max_group);
}

void DiManager::slice_solid_bitmap_relative(uint16_t id, int32_t x, int32_t y, int32_t start_line, int32_t height) {
  DiBitmap* prim; if (!(prim = (DiBitmap*)get_safe_primitive(id))) return;
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
  DiBitmap* prim; if (!(prim = (DiBitmap*)get_safe_primitive(id))) return;
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
  DiBitmap* prim; if (!(prim = (DiBitmap*)get_safe_primitive(id))) return;
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
  DiBitmap* prim; if (!(prim = (DiBitmap*)get_safe_primitive(id))) return;
  int32_t px = x + nth;
  int32_t py = y;
  while (px >= prim->get_width()) {
    px -= prim->get_width();
    py++;
  }
  prim->set_transparent_pixel(px, py, color);
}

void DiManager::set_masked_bitmap_pixel(uint16_t id, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiBitmap* prim; if (!(prim = (DiBitmap*)get_safe_primitive(id))) return;
  int32_t px = x + nth;
  int32_t py = y;
  while (px >= prim->get_width()) {
    px -= prim->get_width();
    py++;
  }
  prim->set_transparent_pixel(px, py, color);
}

void DiManager::set_transparent_bitmap_pixel(uint16_t id, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiBitmap* prim; if (!(prim = (DiBitmap*)get_safe_primitive(id))) return;
  int32_t px = x + nth;
  int32_t py = y;
  while (px >= prim->get_width()) {
    px -= prim->get_width();
    py++;
  }
  prim->set_transparent_pixel(px, py, color);
}

void DiManager::set_solid_bitmap_pixel_for_tile_array(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiTileArray* prim; if (!(prim = (DiTileArray*)get_safe_primitive(id))) return;
  x += nth;
  while (x >= prim->get_width()) {
    x -= prim->get_width();
    y++;
  }
  prim->set_pixel(bm_id, x, y, color);
}

void DiManager::set_masked_bitmap_pixel_for_tile_array(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiTileArray* prim; if (!(prim = (DiTileArray*)get_safe_primitive(id))) return;
  x += nth;
  while (x >= prim->get_width()) {
    x -= prim->get_width();
    y++;
  }
  prim->set_pixel(bm_id, x, y, color);
}

void DiManager::set_transparent_bitmap_pixel_for_tile_array(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiTileArray* prim; if (!(prim = (DiTileArray*)get_safe_primitive(id))) return;
  x += nth;
  while (x >= prim->get_width()) {
    x -= prim->get_width();
    y++;
  }
  prim->set_pixel(bm_id, x, y, color);
}

void DiManager::set_solid_bitmap_pixel_for_tile_map(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiTileMap* prim; if (!(prim = (DiTileMap*)get_safe_primitive(id))) return;
  x += nth;
  while (x >= prim->get_width()) {
    x -= prim->get_width();
    y++;
  }
  prim->set_pixel(bm_id, x, y, color);
}

void DiManager::set_masked_bitmap_pixel_for_tile_map(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiTileMap* prim; if (!(prim = (DiTileMap*)get_safe_primitive(id))) return;
  x += nth;
  while (x >= prim->get_width()) {
    x -= prim->get_width();
    y++;
  }
  prim->set_pixel(bm_id, x, y, color);
}

void DiManager::set_transparent_bitmap_pixel_for_tile_map(uint16_t id, uint16_t bm_id, int32_t x, int32_t y, uint8_t color, int16_t nth) {
  DiTileMap* prim; if (!(prim = (DiTileMap*)get_safe_primitive(id))) return;
  x += nth;
  while (x >= prim->get_width()) {
    x -= prim->get_width();
    y++;
  }
  prim->set_pixel(bm_id, x, y, color);
}

void DiManager::set_tile_array_bitmap_id(uint16_t id, uint16_t col, uint16_t row, uint16_t bm_id) {
  DiTileArray* prim; if (!(prim = (DiTileArray*)get_safe_primitive(id))) return;
  prim->set_tile(col, row, bm_id);
}

void DiManager::set_tile_map_bitmap_id(uint16_t id, uint16_t col, uint16_t row, uint16_t bm_id) {
  DiTileMap* prim; if (!(prim = (DiTileMap*)get_safe_primitive(id))) return;
  prim->set_tile(col, row, bm_id);
}
