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
#include "di_solid_rectangle.h"
#include "di_tile_map.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "soc/i2s_struct.h"
#include "soc/i2s_reg.h"
#include "ESP32Time.h"
//#include "fabgl.h"
#include "HardwareSerial.h"

void default_on_vertical_blank() {}

void default_on_lines_painted() {}

DiManager::DiManager() {
  m_on_vertical_blank_cb = &default_on_vertical_blank;
  m_on_lines_painted_cb = &default_on_lines_painted;

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
    for (int g = 0; g < NUM_VERTICAL_GROUPS; g++) {
        std::vector<DiPrimitive*> * vp = &m_groups[g];
        vp->clear();
    }

    for (int i = FIRST_PRIMITIVE_ID; i <= LAST_PRIMITIVE_ID; i++) {
      if (m_primitives[i]) {
        delete m_primitives[i];
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
    if (m_primitives[prim->get_id()]) {
      delete m_primitives[prim->get_id()];
    }

    parent->attach_child(prim);
    while (parent != m_primitives[ROOT_PRIMITIVE_ID] && !(parent->get_flags() & PRIM_FLAG_CLIP_KIDS)) {
      parent = parent->get_parent();
    }
    prim->compute_absolute_geometry(parent->get_view_x(), parent->get_view_y(),
      parent->get_view_x_extent(), parent->get_view_y_extent());

    if (prim->get_flags() & PRIM_FLAG_PAINT_THIS) {
      int32_t min_group, max_group;
      prim->get_vertical_group_range(min_group, max_group);
      for (int32_t g = min_group; g <= max_group; g++) {
          m_groups[g].push_back(prim);
      }
    }

    m_primitives[prim->get_id()] = prim;
}

void DiManager::delete_primitive(DiPrimitive* prim) {
  if (prim) {
    if (prim->get_flags() & PRIM_FLAG_PAINT_THIS) {
      int32_t min_group, max_group;
      prim->get_vertical_group_range(min_group, max_group);
      for (int32_t g = min_group; g <= max_group; g++) {
        std::vector<DiPrimitive*> * vp = &m_groups[g];
        auto position2 = std::find(vp->begin(), vp->end(), prim);
        if (position2 != vp->end()) {
          vp->erase(position2);
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

/*void DiManager::set_tile_map_position(DiTileMap* tile_map, int32_t x, int32_t y) {
  tile_map->set_position(x, y);
  return;
  int32_t min_group1, max_group1;
  tile_map->get_vertical_group_range(min_group1, max_group1);

  tile_map->set_position(x, y);

  int32_t min_group2, max_group2;
  tile_map->get_vertical_group_range(min_group2, max_group2);

  for (int32_t g = min_group1; g <= max_group1; g++) {
    if (g < min_group2 || g > max_group2) {
      // Remove the primitive from this vertical scan group
      std::vector<DiPrimitive*> * vp = &m_groups[g];
      auto position2 = std::find(vp->begin(), vp->end(), tile_map);
      if (position2 != vp->end()) {
        vp->erase(position2);
      }
    }
  }

  for (int32_t g = min_group2; g <= max_group2; g++) {
    if (g < min_group1 || g > max_group1) {
      // Add the primitive to this vertical scan group
      std::vector<DiPrimitive*> * vp = &m_groups[g];
      vp->push_back(tile_map);
    }
  }
}*/

DiPrimitive* DiManager::create_point(int32_t x, int32_t y, uint8_t color) {
    DiPrimitive* prim = new DiSetPixel(x, y, color);
    add_primitive(prim, m_primitives[ROOT_PRIMITIVE_ID]);
    return prim;
}

DiPrimitive* DiManager::create_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color) {
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

    add_primitive(prim, m_primitives[ROOT_PRIMITIVE_ID]);
    return prim;
}

DiPrimitive* DiManager::create_solid_rectangle(int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color) {
    auto prim = new DiSolidRectangle();
    prim->init_params(x, y, width, height, color);
    add_primitive(prim, m_primitives[ROOT_PRIMITIVE_ID]);
    return prim;
}

DiPrimitive* DiManager::create_triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color) {
    auto prim = new DiGeneralLine();
    prim->init_params(x1, y1, x2, y2, x3, y3, color);
    add_primitive(prim, m_primitives[ROOT_PRIMITIVE_ID]);
    return prim;
}

DiTileMap* DiManager::create_tile_map(int32_t screen_width, int32_t screen_height,
            uint32_t bitmaps, uint32_t columns, uint32_t rows,
            uint32_t width, uint32_t height, bool hscroll) {
    DiTileMap* tile_map =
      new DiTileMap(screen_width, screen_height, bitmaps, columns, rows, width, height, hscroll);
    add_primitive(tile_map, m_primitives[ROOT_PRIMITIVE_ID]);
    return tile_map;
}

DiTerminal* DiManager::create_terminal(uint32_t x, uint32_t y, uint32_t codes, uint32_t columns, uint32_t rows,
                            uint8_t fg_color, uint8_t bg_color, const uint8_t* font) {
    DiTerminal* terminal = new DiTerminal(x, y, codes, columns, rows, fg_color, bg_color, font);
    add_primitive(terminal, m_primitives[ROOT_PRIMITIVE_ID]);
    return terminal;
}

void IRAM_ATTR DiManager::run() {
  initialize();
  loop();
  clear();
}

void IRAM_ATTR DiManager::loop() {
  DiPaintParams paint_params;
  paint_params.m_horiz_scroll = 0;
  paint_params.m_vert_scroll = 0;
  paint_params.m_screen_width = ACT_PIXELS;
  paint_params.m_screen_height = ACT_LINES;

  uint32_t current_line_index = 0;//NUM_ACTIVE_BUFFERS * NUM_LINES_PER_BUFFER;
  uint32_t current_buffer_index = 0;
  bool end_of_frame = false;

  while (true) {
    uint32_t descr_addr = (uint32_t) I2S1.out_link_dscr;
    uint32_t descr_index = (descr_addr - (uint32_t)m_dma_descriptor) / sizeof(lldesc_t);
    if (descr_index <= ACT_BUFFERS_WRITTEN) {
      //uint32_t dma_line_index = descr_index * NUM_LINES_PER_BUFFER;
      uint32_t dma_buffer_index = descr_index & (NUM_ACTIVE_BUFFERS-1);

      // Draw enough lines to stay ahead of DMA.
      while (current_line_index < ACT_LINES && current_buffer_index != dma_buffer_index) {
        volatile DiVideoBuffer* vbuf = &m_video_buffer[current_buffer_index];
        paint_params.m_line_index = current_line_index;
        paint_params.m_scrolled_y = current_line_index + paint_params.m_vert_scroll;
        paint_params.m_line8 = (volatile uint8_t*) vbuf->get_buffer_ptr_0();
        paint_params.m_line32 = vbuf->get_buffer_ptr_0();
        draw_primitives(&paint_params);

        paint_params.m_line_index = ++current_line_index;
        paint_params.m_scrolled_y = current_line_index + paint_params.m_vert_scroll;
        paint_params.m_line8 = (volatile uint8_t*) vbuf->get_buffer_ptr_1();
        paint_params.m_line32 = vbuf->get_buffer_ptr_1();
        draw_primitives(&paint_params);

        ++current_line_index;
        if (++current_buffer_index >= NUM_ACTIVE_BUFFERS) {
          current_buffer_index = 0;
        }
      }
      end_of_frame = false;
      (*m_on_lines_painted_cb)();
    } else if (!end_of_frame) {
      // Handle modifying primitives before the next frame starts.
      on_vertical_blank();

      // Prepare the start of the next frame.
      for (current_line_index = 0, current_buffer_index = 0;
            current_buffer_index < NUM_ACTIVE_BUFFERS;
            current_line_index++, current_buffer_index++) {
        volatile DiVideoBuffer* vbuf = &m_video_buffer[current_buffer_index];
        paint_params.m_line_index = current_line_index;
        paint_params.m_scrolled_y = current_line_index + paint_params.m_vert_scroll;
        paint_params.m_line8 = (volatile uint8_t*) vbuf->get_buffer_ptr_0();
        paint_params.m_line32 = vbuf->get_buffer_ptr_0();
        draw_primitives(&paint_params);

        paint_params.m_line_index = ++current_line_index;
        paint_params.m_scrolled_y = current_line_index + paint_params.m_vert_scroll;
        paint_params.m_line8 = (volatile uint8_t*) vbuf->get_buffer_ptr_1();
        paint_params.m_line32 = vbuf->get_buffer_ptr_1();
        draw_primitives(&paint_params);
      }

      end_of_frame = true;
      current_line_index = 0;
      current_buffer_index = 0;
    }
  }
}

void IRAM_ATTR DiManager::draw_primitives(DiPaintParams* params) {
  int32_t g = params->m_line_index >> VERTICAL_GROUP_INDEX_SHIFT;
  std::vector<DiPrimitive*> * vp = &m_groups[g];
  for (auto prim = vp->begin(); prim != vp->end(); ++prim) {
      (*prim)->paint(params);
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

void IRAM_ATTR DiManager::on_vertical_blank() {
  (*m_on_vertical_blank_cb)();
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
