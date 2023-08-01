// di_video_buffer.cpp - Function definitions for painting video scan lines
//
// A a video buffer is a set of 1-pixel-high video scan lines that are equal
// in length (number of pixels) to the total width of the video screen and
// horizontal synchronization pixels.
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

#include <string.h>
#include <math.h>
#include <vector>
#include "di_video_buffer.h"

void DiVideoScanLine::init_to_black() volatile {
  memset((void*)m_act, SYNCS_OFF, ACT_PIXELS);
  memset((void*)m_hfp, SYNCS_OFF, HFP_PIXELS);
  memset((void*)m_hs, (HSYNC_ON|VSYNC_OFF), HS_PIXELS);
  memset((void*)m_hbp, SYNCS_OFF, HBP_PIXELS);
}

void DiVideoScanLine::init_for_vsync() volatile {
  memset((void*)m_act, (HSYNC_OFF|VSYNC_ON), ACT_PIXELS);
  memset((void*)m_hfp, (HSYNC_OFF|VSYNC_ON), HFP_PIXELS);
  memset((void*)m_hs, SYNCS_ON, HS_PIXELS);
  memset((void*)m_hbp, (HSYNC_OFF|VSYNC_ON), HBP_PIXELS);
}

void DiVideoBuffer::init_to_black() volatile {
  for (int i = 0; i < NUM_LINES_PER_BUFFER; i++) {
    m_line[i].init_to_black();
  }
}

void DiVideoBuffer::init_for_vsync() volatile {
  for (int i = 0; i < NUM_LINES_PER_BUFFER; i++) {
    m_line[i].init_for_vsync();
  }
}
