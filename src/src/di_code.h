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
typedef void (*CallEspFcn)(void* p_this, volatile uint32_t* p_scan_line, uint32_t line_index);
};

class EspFunction {
    public:
    EspFunction();
    ~EspFunction();

    void d8(uint32_t value) { add8(value); }
    void d16(uint32_t value) { add16(value); }
    void d24(uint32_t value) { add24(value); }
    void d32(uint32_t value) { add32(value); }
    void entry(reg_t src, u_off_t offset) { add24(iso(0x000036, src, offset)); }
    void j(s_off_t offset) { add24(io(0x000006, offset)); }
    void l16si(reg_t dst, reg_t src, u_off_t offset) { add24(idso(0x009002, dst, src, offset)); }
    void l16ui(reg_t dst, reg_t src, u_off_t offset) { add24(idso(0x001002, dst, src, offset)); }
    void l32i(reg_t dst, reg_t src, u_off_t offset) { add24(idso(0x002002, dst, src, offset)); }
    void l32r(reg_t dst, s_off_t offset) { add24(ido(0x000001, dst, offset)); }
    void l8ui(reg_t dst, reg_t src, u_off_t offset) { add24(idso(0x000002, dst, src, offset)); }
    void movi(reg_t dst, uint32_t value) { add24(iv(0x00A002, dst, value)); }
    void ret() { add24(0x000080); }
    void retw() { add24(0x000090); }
    void s16i(reg_t dst, reg_t src, u_off_t offset) { add24(idso(0x005002, dst, src, offset)); }
    void s32i(reg_t dst, reg_t src, u_off_t offset) { add24(idso(0x006002, dst, src, offset)); }
    void s8i(reg_t dst, reg_t src, u_off_t offset) { add24(idso(0x004002, dst, src, offset)); }

    inline void call(void* p_this, volatile uint32_t* p_scan_line, uint32_t line_index) {
        (*((CallEspFcn)m_code))(p_this, p_scan_line, line_index);
    }

    protected:
    uint32_t    m_alloc_size;
    uint32_t    m_code_size;
    uint32_t*   m_code;

    void allocate(uint32_t size);
    void store(uint8_t instr_byte);
    void add8(instr_t data);
    void add16(instr_t data);
    void add24(instr_t data);
    void add32(instr_t data);

    inline instr_t idso(uint32_t instr, reg_t dst, reg_t src, u_off_t offset) {
        return instr | ((offset >> 1) << 16) | (dst << 4) | (src << 8); }

    inline instr_t iso(uint32_t instr, reg_t src, u_off_t offset) {
        return instr | ((offset >> 3) << 12) | (src << 8); }

    inline instr_t ido(uint32_t instr, reg_t dst, u_off_t offset) {
        return instr | ((offset >> 2) << 8) | (dst << 4); }

    inline instr_t io(uint32_t instr, u_off_t offset) {
        return instr | (offset << 6); }

    inline instr_t iv(uint32_t instr, reg_t dst, uint32_t value) {
        return instr | ((value & 0xFF) << 16) | (dst << 4) | (value & 0xF00); }

};
