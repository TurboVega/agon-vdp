// di_code.cpp - Supports creating ESP32 functions dynamically.
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

#include "di_code.h"
#include "../agon.h"
#include "freertos/FreeRTOS.h"
#include <string.h>

#define EXTRA_CODE_SIZE 8

// Input registers:
#define REG_RETURN_ADDR     a0
#define REG_STACK_PTR       a1
#define REG_THIS_PTR        a2
#define REG_LINE_PTR        a3
#define REG_LINE_INDEX      a4
// Temporary registers:
#define REG_DST_PIXEL_PTR   a5
#define REG_SRC_PIXEL_PTR   a6
#define REG_PIXEL_COLOR     a7
#define REG_DRAW_WIDTH      a8
#define REG_LOOP_INDEX      a9

#define FIX_OFFSET(off)    ((off)^2)

EspFunction::EspFunction() {
    m_alloc_size = 0;
    m_code_size = 0;
    m_code_index = 0;
    m_code = 0;
}

EspFunction::~EspFunction() {
    if (m_code) {
        heap_caps_free(m_code);
    }
}

// Ex: X=0, w=1: b[2]=c
void EspFunction::set_1_pixel_at_offset_0() {
    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
}

// Ex: X=1, w=1: b[3]=c
void EspFunction::set_1_pixel_at_offset_1() {
    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));
}

// Ex: X=2, w=1: b[0]=c
void EspFunction::set_1_pixel_at_offset_2() {
    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
}

// Ex: X=3, w=1: b[1]=c
void EspFunction::set_1_pixel_at_offset_3() {
    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(3));
}

// Ex: X=0, w=2: h[2]=c1c0
void EspFunction::set_2_pixels_at_offset_0() {
    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
}

// Ex: X=1, w=2: b[3]=c0; b[0]=c1
void EspFunction::set_2_pixels_at_offset_1() {
    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));
    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
}

// Ex: X=2, w=2: h[0]=c1c0
void EspFunction::set_2_pixels_at_offset_2() {
    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));    
}

// Ex: X=0, w=3: h[2]=c1c0; b[0]=c2
void EspFunction::set_3_pixels_at_offset_0() {
    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(0));
    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
}

// Ex: X=1, w=3: b[3]=c0; h[0]=c2c1
void EspFunction::set_3_pixels_at_offset_1() {
    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(1));    
    s16i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, FIX_OFFSET(2));
}

// Ex: X=0, w=4: w=c1c0c3c2
void EspFunction::set_4_pixels_at_offset(u_off_t offset) {
    s32i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, offset);
}

// Ex: X1=27, width=55, color=0x03030303
void EspFunction::draw_line(uint32_t x, uint32_t width, uint32_t color) {
    auto at_jump = enter_function();
    auto at_data = begin_data();
    auto aligned_x = x & 0xFFFFFFFC;
    auto at_x = write32(aligned_x);
    auto at_color = write32(color);

    begin_code(at_jump);

    set_reg_dst_pixel_ptr(at_x);
    set_reg_color(at_color);

    while (width) {
        auto offset = x & 3;
        uint32_t sub = 1;
        switch (offset) {
            case 0:
                if (width >= 4) {
                    if (width >= 128) {
                        // Need at least 32 full words
                        auto times = width / 64;
                        movi(REG_LOOP_INDEX, times);
                        auto at_loop = get_pc();
                        loop(REG_LOOP_INDEX, 0);
                        set_4_pixels_at_offset(0);
                        set_4_pixels_at_offset(4);
                        set_4_pixels_at_offset(8);
                        set_4_pixels_at_offset(12);
                        set_4_pixels_at_offset(16);
                        set_4_pixels_at_offset(20);
                        set_4_pixels_at_offset(24);
                        set_4_pixels_at_offset(28);
                        set_4_pixels_at_offset(32);
                        set_4_pixels_at_offset(36);
                        set_4_pixels_at_offset(40);
                        set_4_pixels_at_offset(44);
                        set_4_pixels_at_offset(48);
                        set_4_pixels_at_offset(52);
                        set_4_pixels_at_offset(56);
                        set_4_pixels_at_offset(60);
                        addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 64);

                        uint32_t save_pc = get_pc();
                        set_pc(at_loop);
                        loop(REG_LOOP_INDEX, save_pc - (at_loop + 4));
                        set_pc(save_pc);

                        sub = times * 64;
                        width -= sub;
                        x += sub;
                        continue;
                    } else if (width >= 64) {
                        // Need at least 16 full words
                        set_4_pixels_at_offset(0);
                        set_4_pixels_at_offset(4);
                        set_4_pixels_at_offset(8);
                        set_4_pixels_at_offset(12);
                        set_4_pixels_at_offset(16);
                        set_4_pixels_at_offset(20);
                        set_4_pixels_at_offset(24);
                        set_4_pixels_at_offset(28);
                        set_4_pixels_at_offset(32);
                        set_4_pixels_at_offset(36);
                        set_4_pixels_at_offset(40);
                        set_4_pixels_at_offset(44);
                        set_4_pixels_at_offset(48);
                        set_4_pixels_at_offset(52);
                        set_4_pixels_at_offset(56);
                        set_4_pixels_at_offset(60);
                        sub = 64;
                    } else if (width >= 32) {
                        // Need at least 8 full words
                        set_4_pixels_at_offset(0);
                        set_4_pixels_at_offset(4);
                        set_4_pixels_at_offset(8);
                        set_4_pixels_at_offset(12);
                        set_4_pixels_at_offset(16);
                        set_4_pixels_at_offset(20);
                        set_4_pixels_at_offset(24);
                        set_4_pixels_at_offset(28);
                        sub = 32;
                    } else if (width >= 16) {
                        // Need at least 4 full words
                        set_4_pixels_at_offset(0);
                        set_4_pixels_at_offset(4);
                        set_4_pixels_at_offset(8);
                        set_4_pixels_at_offset(12);
                        sub = 16;
                    } else if (width >= 8) {
                        // Need at least 2 full words
                        set_4_pixels_at_offset(0);
                        set_4_pixels_at_offset(4);
                        sub = 8;
                    } else {
                        // Need at least 1 full word
                        set_4_pixels_at_offset(0);
                        sub = 4;
                    }

                    width -= sub;
                    x += sub;
                    if (width) {
                        addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, sub);
                    }
                    continue;
                } else if (width == 3) {
                    set_3_pixels_at_offset_0();
                    sub = 3;
                } else if (width == 2) {
                    set_2_pixels_at_offset_0();
                    sub = 2;
                } else /* width == 1 */ {
                    set_1_pixel_at_offset_0();
                }
                break;

            case 1:
                if (width >= 3) {
                    set_3_pixels_at_offset_1();
                    sub = 3;                
                } else if (width == 2) {
                    set_2_pixels_at_offset_1();
                    sub = 2;
                } else /* width == 1 */ {
                    set_1_pixel_at_offset_1();
                }
                break;

            case 2:
                if (width >= 2) {
                    set_2_pixels_at_offset_2();
                    sub = 2;
                } else /* width == 1 */ {
                    set_1_pixel_at_offset_2();
                }
                break;
            
            case 3:
                set_1_pixel_at_offset_3();
                break;
        }
        width -= sub;
        x += sub;
        if (width) {
            addi(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, 4);
        }
    }
    leave_function();
}

uint32_t EspFunction::enter_function() {
    entry(sp, 16);
    auto at_jump = get_pc();
    j(0);
    return at_jump;
}

void EspFunction::leave_function() {
    retw();
}

uint32_t EspFunction::begin_data() {
    align32();
    return get_pc();
}

void EspFunction::begin_code(uint32_t at_jump) {
    align32();
    j_to_here(at_jump);
}

void EspFunction::set_reg_draw_width(uint32_t at_width) {
    l32r_from(REG_DRAW_WIDTH, at_width);
}

void EspFunction::set_reg_dst_pixel_ptr(uint32_t at_x) {
    l32r_from(REG_DST_PIXEL_PTR, at_x);
    add(REG_DST_PIXEL_PTR, REG_DST_PIXEL_PTR, REG_LINE_PTR);
}

void EspFunction::set_reg_color(uint32_t at_color) {
    l32r_from(REG_PIXEL_COLOR, at_color);
}

void EspFunction::store(uint8_t instr_byte) {
    auto i = m_code_index >> 2;
    switch (m_code_index & 3) {
        case 0:
            m_code[i] = (m_code[i] & 0xFFFFFF00) | (uint32_t)instr_byte;
            break;
        case 1:
            m_code[i] = (m_code[i] & 0xFFFF00FF) |((uint32_t)instr_byte) << 8;
            break;
        case 2:
            m_code[i] = (m_code[i] & 0xFF00FFFF) |((uint32_t)instr_byte) << 16;
            break;
        case 3:
            m_code[i] = (m_code[i] & 0x00FFFFFF) |((uint32_t)instr_byte) << 24;
            break;
    }

    if (++m_code_index > m_code_size) {
        m_code_size = m_code_index;
    }
}

void EspFunction::align16() {
    if (m_code_index & 1) {
        write8(0);
    }
}

void EspFunction::align32() {
    align16();
    if (m_code_index & 2) {
        write16(0);
    }
}

void EspFunction::j_to_here(uint32_t from) {
    uint32_t save_pc = get_pc();
    set_pc(from);
    j(save_pc - from - 4);
    set_pc(save_pc);
}

void EspFunction::l32r_from(reg_t reg, uint32_t from) {
    l32r(reg, from - ((get_pc() + 3) & 0xFFFFFFFC));
}

uint16_t EspFunction::dup8_to_16(uint8_t value) {
    return (((uint16_t)value) << 8) | ((uint16_t)value);
}

uint32_t EspFunction::dup8_to_32(uint8_t value) {
    return (dup8_to_16(value) << 16) | dup8_to_16(value);
}

uint32_t EspFunction::dup16_to_32(uint16_t value) {
    return (((uint32_t)value) << 16) | ((uint32_t)value);
}

void EspFunction::allocate(uint32_t size) {
    if (m_alloc_size) {
        if (m_alloc_size - m_code_index < size) {
            size_t new_size = (size_t)(m_alloc_size + size + EXTRA_CODE_SIZE + 3) &0xFFFFFFFC;
            void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_EXEC);
            memcpy(p, m_code, (m_code_size + 3) &0xFFFFFFFC);
            heap_caps_free(m_code);
            m_alloc_size = (uint32_t)new_size;
            m_code = (uint32_t*)p;
        }
    } else {
        size_t new_size = (size_t)(size + EXTRA_CODE_SIZE);
        void* p = heap_caps_malloc(new_size, MALLOC_CAP_32BIT|MALLOC_CAP_EXEC);
        m_alloc_size = (uint32_t)new_size;
        m_code = (uint32_t*)p;
    }
}

uint32_t EspFunction::write8(instr_t data) {
    allocate(1);
    auto at_data = get_pc();
    store((uint8_t)(data & 0xFF));
    return at_data;
}

uint32_t EspFunction::write16(instr_t data) {
    allocate(2);
    auto at_data = get_pc();
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
    return at_data;
}

uint32_t EspFunction::write24(instr_t data) {
    allocate(3);
    auto at_data = get_pc();
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
    store((uint8_t)((data >> 16) & 0xFF));
    return at_data;
}

uint32_t EspFunction::write32(instr_t data) {
    allocate(4);
    auto at_data = get_pc();
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
    store((uint8_t)((data >> 16) & 0xFF));
    store((uint8_t)((data >> 24) & 0xFF));
    return at_data;
}

instr_t isieo(uint32_t instr, reg_t src, int32_t imm, u_off_t offset) {
    if (imm == -1) imm = 0;
    else if (imm == 10) imm = 9;
    else if (imm == 12) imm = 10;
    else if (imm == 16) imm = 11;
    else if (imm == 32) imm = 12;
    else if (imm == 64) imm = 13;
    else if (imm == 128) imm = 14;
    else if (imm == 256) imm = 15;
    return instr | (offset << 16) | (imm << 12) | (src << 8);
}
