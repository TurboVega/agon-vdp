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

/*
    entry(sp, 16);
    uint32_t at_jump = get_pc();
    j(0);
    align32();
    uint32_t at_data = get_pc();
    d32(color);
    j_to_here(at_jump);
    l32r_from(REG_PIXEL_COLOR, at_data);
    s8i(REG_PIXEL_COLOR, a3, offset);
    retw();
*/

uint32_t EspFunction::set_pixel_at_byte_offset(uint32_t offset, uint8_t color) {
    movi(REG_DST_PIXEL_PTR, REG_LINE_PTR);
    movi(REG_PIXEL_COLOR, color);
    s8i(REG_PIXEL_COLOR, REG_DST_PIXEL_PTR, 0);
    addi(REG_DST_PIXEL_PTR, 1);
    return ++offset;
}

uint32_t EspFunction::set_pixel_pair_at_byte_offset(uint32_t offset, uint8_t color) {
    
}

uint32_t EspFunction::set_pixel_pair_at_byte_offset(uint32_t offset, uint8_t color0, uint8_t color1) {
    
}

uint32_t EspFunction::set_pixel_at_x_offset_0(uint32_t x, uint8_t color) {
    
}

uint32_t EspFunction::set_pixel_at_x_offset_1(uint32_t x, uint8_t color) {
    
}

uint32_t EspFunction::set_pixel_at_x_offset_2(uint32_t x, uint8_t color) {
    
}

uint32_t EspFunction::set_pixel_at_x_offset_3(uint32_t x, uint8_t color) {
    
}

uint32_t EspFunction::set_pixel_pair_at_x_offset_0(uint32_t x, uint8_t color) {
    
}

uint32_t EspFunction::set_pixel_pair_at_x_offset_2(uint32_t x, uint8_t color) {
    
}

uint32_t EspFunction::set_pixel_pair_at_x_offset_0(uint32_t x, uint8_t color0, uint8_t color1) {
    
}

uint32_t EspFunction::set_pixel_pair_at_x_offset_2(uint32_t x, uint8_t color0, uint8_t color1) {
    
}

uint32_t EspFunction::set_pixel_quad(uint32_t x) {

}

uint32_t EspFunction::set_pixel_quad(uint32_t x, uint8_t color) {
    
}

uint32_t EspFunction::set_pixel_quads(uint32_t x, uint32_t count) {
    
}

uint32_t EspFunction::set_pixel_quads(uint32_t x, uint8_t color, uint32_t count) {
    
}

uint32_t EspFunction::draw_line(uint32_t x1, uint32_t x2, uint8_t color) {

}

void init_output_ptr(uint32_t x) {
    f.movi(a5, )
}

void EspFunction::store(uint8_t instr_byte) {
    uint32_t i = m_code_index >> 2;
    switch (m_code_index & 3) {
        case 0:
            m_code[i] = (uint32_t)instr_byte;
            break;
        case 1:
            m_code[i] |= ((uint32_t)instr_byte) << 8;
            break;
        case 2:
            m_code[i] |= ((uint32_t)instr_byte) << 16;
            break;
        case 3:
            m_code[i] |= ((uint32_t)instr_byte) << 24;
            break;
    }

    if (++m_code_index > m_code_size) {
        m_code_size = m_code_index;
    }
}

void EspFunction::align16() {
    if (m_code_index & 1) {
        add8(0);
    }
}

void EspFunction::align32() {
    while (m_code_index & 3) {
        align16();
    }
}

void EspFunction::j_to_here(uint32_t from) {
    j(m_code_index - from - 4);
}

void EspFunction::l32r_from(uint32_t from) {
    l32r(a5, 8-((12+3)&0xFFFFFFFC));
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

void EspFunction::add8(instr_t data) {
    allocate(1);
    store((uint8_t)(data & 0xFF));
}

void EspFunction::add16(instr_t data) {
    allocate(2);
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
}

void EspFunction::add24(instr_t data) {
    allocate(3);
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
    store((uint8_t)((data >> 16) & 0xFF));
}

void EspFunction::add32(instr_t data) {
    allocate(4);
    store((uint8_t)(data & 0xFF));
    store((uint8_t)((data >> 8) & 0xFF));
    store((uint8_t)((data >> 16) & 0xFF));
    store((uint8_t)((data >> 24) & 0xFF));
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
