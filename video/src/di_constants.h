// di_constants.h - Constants used in agon_crayon files.
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

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define ABS(X) (((X) >= 0) ? (X) : (-(X)))

#define GPIO_RED_0    GPIO_NUM_21
#define GPIO_RED_1    GPIO_NUM_22
#define GPIO_GREEN_0  GPIO_NUM_18
#define GPIO_GREEN_1  GPIO_NUM_19
#define GPIO_BLUE_0   GPIO_NUM_4
#define GPIO_BLUE_1   GPIO_NUM_5
#define GPIO_HSYNC    GPIO_NUM_23
#define GPIO_VSYNC    GPIO_NUM_15

#define VS0 0
#define VS1 1
#define HS0 0
#define HS1 1
#define R0  0
#define R1  1
#define R2  2
#define R3  3
#define G0  0
#define G1  1
#define G2  2
#define G3  3
#define B0  0
#define B1  1
#define B2  2
#define B3  3

#define VGA_RED_BIT    0
#define VGA_GREEN_BIT  2
#define VGA_BLUE_BIT   4
#define VGA_HSYNC_BIT  6
#define VGA_VSYNC_BIT  7

#define HSYNC_ON  (1<<VGA_HSYNC_BIT)
#define HSYNC_OFF (0<<VGA_HSYNC_BIT)
#define VSYNC_ON  (1<<VGA_VSYNC_BIT)
#define VSYNC_OFF (0<<VGA_VSYNC_BIT)
#define SYNCS_ON  (HSYNC_ON|VSYNC_ON)
#define SYNCS_OFF (HSYNC_OFF|VSYNC_OFF)
#define SYNCS_OFF_32  ((uint32_t)SYNCS_OFF)
#define SYNCS_OFF_X4  ((SYNCS_OFF_32 << 24)|(SYNCS_OFF_32 << 16)|(SYNCS_OFF_32 << 8)|(SYNCS_OFF_32))

#define MASK_RGB(r,g,b) (((r)<<VGA_RED_BIT)|((g)<<VGA_GREEN_BIT)|((b)<<VGA_BLUE_BIT))

#define ACT_LINES     600   // visible lines
#define VFP_LINES     1     // vertical front porch lines
#define VS_LINES      4     // vertical sync lines
#define VBP_LINES     23    // vertical back porch lines

#define HFP_PIXELS    40    // horizontal front porch pixels
#define HS_PIXELS     128   // horizontal sync pixels
#define ACT_PIXELS    800   // visible pixels
#define HBP_PIXELS    88    // horizontal back porch pixels

#define DMA_CLOCK_FREQ ((uint32_t)40000000) // 40 MHz

// Used by certain test code to show diamonds.
#define CENTER_X            (ACT_PIXELS/2)
#define CENTER_Y            (ACT_LINES/2)
#define DIAMOND_SIZE        400
#define HALF_DIAMOND_SIZE   (DIAMOND_SIZE/2)
#define DIAMOND_START_LINE  (CENTER_Y-HALF_DIAMOND_SIZE)
#define DIAMOND_END_LINE    (DIAMOND_START_LINE+DIAMOND_SIZE)

#define SMALL_DIAMOND_SIZE  100
#define HALF_SMALL_DIAMOND_SIZE (SMALL_DIAMOND_SIZE/2)

#define TINY_DIAMOND_SIZE  50
#define HALF_TINY_DIAMOND_SIZE (TINY_DIAMOND_SIZE/2)

// This formula handles arranging the pixel bytes in the correct DMA order.
// 0x12345678, normally stored as 78, 56, 34, 12, is sent as 34 12 78 56.
#define FIX_INDEX(idx)      ((idx)^2)

// Used to control the few DMA scan line buffers.
#define NUM_LINES_PER_BUFFER  2
#define NUM_ACTIVE_BUFFERS    4 // must be a power of 2 and multiple of NUM_LINES_PER_BUFFER
#define ACT_BUFFERS_WRITTEN   (ACT_LINES/NUM_LINES_PER_BUFFER)
#define VS_BUFFERS_WRITTEN    (VS_LINES/NUM_LINES_PER_BUFFER)

#define DMA_TOTAL_LINES       (ACT_LINES+VFP_LINES+VS_LINES+VBP_LINES)
#define DMA_TOTAL_DESCR       (ACT_BUFFERS_WRITTEN+VFP_LINES+VS_BUFFERS_WRITTEN+VBP_LINES)

// Used to somewhat optimize which top-level primitives are called
// to paint, relative to the current scan line.
#define VERTICAL_GROUP_INDEX_SHIFT 4 // used to divide by a power of 2
#define NUM_VERTICAL_GROUPS ((ACT_LINES >> VERTICAL_GROUP_INDEX_SHIFT) + 1)
