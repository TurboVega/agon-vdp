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

typedef void (*DiVoidCallback)();

class DiManager {
    public:
    // Construct a drawing-instruction manager, which handles multiple drawing primitives.
    DiManager();

    // Destroy the manager and its child primitives.
    ~DiManager();

    // Create various types of drawing primitives.
    DiPrimitive* create_point(int32_t x, int32_t y, uint8_t color);
    DiPrimitive* create_line(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color);
    DiPrimitive* create_solid_rectangle(int32_t x, int32_t y, uint32_t width, uint32_t height, uint8_t color);
    DiPrimitive* create_triangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, uint8_t color);
    DiTileMap* create_tile_map(int32_t screen_width, int32_t screen_height, uint32_t bitmaps,
                                uint32_t columns, uint32_t rows, uint32_t width, uint32_t height, bool hscroll);
    DiTerminal* create_terminal(uint32_t x, uint32_t y, uint32_t codes, uint32_t columns, uint32_t rows,
                                uint8_t fg_color, uint8_t bg_color, const uint8_t* font);

    // Setup a callback for when the visible frame pixels have been sent to DMA,
    // and the vertical blanking time begins.
    void set_on_vertical_blank_cb(DiVoidCallback callback_fcn);

    // Setup a callback for when a pair of visible scan lines has been painted,
    // before the next lines are painted.
    void set_on_lines_painted_cb(DiVoidCallback callback_fcn);

    // Setup and run the main loop to do continuous drawing.
    // For the demo, the loop never ends.
    void IRAM_ATTR run();

    // Set the position of a top-level tile map.
    //void set_tile_map_position(DiTileMap* tile_map, int32_t x, int32_t y);

    protected:
    // Structures used to support DMA for video.
    volatile lldesc_t *         m_dma_descriptor; // [DMA_TOTAL_DESCR]
    volatile DiVideoBuffer *    m_video_buffer; // [NUM_ACTIVE_BUFFERS]
    volatile DiVideoScanLine *  m_front_porch;
    volatile DiVideoBuffer *    m_vertical_sync;
    volatile DiVideoScanLine *  m_back_porch;
    DiVoidCallback              m_on_vertical_blank_cb;
    DiVoidCallback              m_on_lines_painted_cb;

    DiPrimitive *               m_primitives[MAX_NUM_PRIMITIVES]; // Indexes of array are primitive IDs
    std::vector<DiPrimitive*>   m_groups[NUM_VERTICAL_GROUPS]; // Vertical scan groups (for optimizing paint calls)

    // Setup the DMA stuff.
    void initialize();

    // Run the main loop.
    void IRAM_ATTR loop();

    // Clear the primitive data, etc.
    void clear();

    // Add a primitive to the manager.
    void add_primitive(DiPrimitive* prim, DiPrimitive* parent);

    // Delete a primitive from the manager.
    void delete_primitive(DiPrimitive* prim);

    // Draw all primitives that belong to the active scan line group.
    void IRAM_ATTR draw_primitives(DiPaintParams* params);

    // Update things like X,Y coordinates, adding/removing primitives, etc.
    void IRAM_ATTR on_vertical_blank();

    // Setup a single DMA descriptor.
    void init_dma_descriptor(volatile DiVideoScanLine* vbuf, uint32_t descr_index);

    // Setup a pair of DMA descriptors.
    void init_dma_descriptor(volatile DiVideoBuffer* vbuf, uint32_t descr_index);
};
