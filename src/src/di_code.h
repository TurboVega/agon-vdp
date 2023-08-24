// di_code.h - Supports creating ESP32 functions dynamically.
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

#include <stdint.h>

typedef enum {
    a0 = 0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15,
    ra = 0, sp = 1
} reg_t;

typedef uint32_t instr_t;   // instruction
typedef uint32_t u_off_t;     // unsigned offset
typedef int32_t s_off_t;   // signed offset

extern "C" {
typedef void (*CallEspFcn)(void* p_this, void* p_params);
};

class EspFunction {
    public:
    EspFunction();
    ~EspFunction();

    void entry(reg_t src, u_off_t offset) {
        add(instr_entry(src, offset));
    }

    void l8ui(reg_t dst, reg_t src, u_off_t offset) {
        add(instr_l8ui(dst, src, offset));
    }

    void l16ui(reg_t dst, reg_t src, u_off_t offset) {
        add(instr_l16ui(dst, src, offset));
    }

    void l16si(reg_t dst, reg_t src, u_off_t offset) {
        add(instr_l16si(dst, src, offset));
    }

    void l32i(reg_t dst, reg_t src, u_off_t offset) {
        add(instr_l32i(dst, src, offset));
    }

    void l32r(reg_t dst, s_off_t offset) {
        add(instr_l32r(dst, offset));
    }

    void s8i(reg_t dst, reg_t src, u_off_t offset) {
        add(instr_s8i(dst, src, offset));
    }

    void s16i(reg_t dst, reg_t src, u_off_t offset) {
        add(instr_s16i(dst, src, offset));
    }

    void s32i(reg_t dst, reg_t src, u_off_t offset) {
        add(instr_s32i(dst, src, offset));
    }

    void movi(reg_t dst, uint32_t value) {
        add(instr_movi(dst, value));
    }

    void ret() {
        add(instr_ret());
    }

    void retw() {
        add(instr_retw());
    }

    inline void call(void* p_this, void* p_params) {
        (*((CallEspFcn)m_code))(p_this, p_params);
    }

    protected:
    uint32_t    m_alloc_size;
    uint32_t    m_code_size;
    uint32_t*   m_code;

    void allocate(uint32_t size);
    void store(uint8_t instr_byte);
    void add(instr_t instruction);
    void add_n(instr_t instruction);

    inline instr_t instr_entry(reg_t src, u_off_t offset) {
        return 0x000036 | ((offset >> 3) << 12) | (src << 8);
    }

    inline instr_t instr_l8ui(reg_t dst, reg_t src, u_off_t offset) {
        return 0x000002 | (offset << 16) | (dst << 4) | (src << 8);
    }

    inline instr_t instr_l16ui(reg_t dst, reg_t src, u_off_t offset) {
        return 0x001002 | ((offset >> 1) << 16) | (dst << 4) | (src << 8);
    }

    inline instr_t instr_l16si(reg_t dst, reg_t src, u_off_t offset) {
        return 0x009002 | ((offset >> 1) << 16) | (dst << 4) | (src << 8);
    }

    inline instr_t instr_l32i(reg_t dst, reg_t src, u_off_t offset) {
        return 0x002002 | ((offset >> 2) << 16) | (dst << 4) | (src << 8);
    }

    inline instr_t instr_l32r(reg_t dst, s_off_t offset) {
        return 0x000001 | ((offset >> 2) << 16) | (dst << 4);
    }

    inline instr_t instr_s8i(reg_t dst, reg_t src, u_off_t offset) {
        return 0x005002 | (offset << 16) | (dst << 4) | (src << 8);
    }

    inline instr_t instr_s16i(reg_t dst, reg_t src, u_off_t offset) {
        return 0x005002 | ((offset >> 1) << 16) | (dst << 4) | (src << 8);
    }

    inline instr_t instr_s32i(reg_t dst, reg_t src, u_off_t offset) {
        return 0x006002 | ((offset >> 2) << 16) | (dst << 4) | (src << 8);
    }

    inline instr_t instr_movi(reg_t dst, uint32_t value) {
        return 0x00A002 | ((value & 0xFF) << 16) | (dst << 4) | (value & 0xF00);
    }

    inline instr_t instr_ret() {
        return 0x000080;
    }

    inline instr_t instr_retw() {
        return 0x000090;
    }

};
