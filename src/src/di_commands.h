// di_commands.h - Data structures of VDU serial commands.
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

#pragma pack(push, 1)

typedef struct {
    uint8_t     m_category;
    uint8_t     m_subcategory;
    uint8_t     m_command;
} VduHeader;

typedef struct {
    uint16_t    m_x;
    uint16_t    m_y;
} VduCoords;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_flags;
} VduOtfSetFlagsForPrimitive;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
} VduOtfSetPrimitivePosition;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
} VduOtfAdjustPrimitivePosition;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
} VduOtfDeletePrimitive;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfCreatePrimitivePoint;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords[2];
    uint8_t     m_color;
} VduOtfCreatePrimitiveLine;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords[3];
    uint8_t     m_color;
} VduOtfCreatePrimitiveTriangleOutline;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords[3];
    uint8_t     m_color;
} VduOtfCreatePrimitiveSolidTriangle;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreatePrimitiveRectangleOutline;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreatePrimitiveSolidRectangle;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreatePrimitiveEllipseOutline;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreatePrimitiveSolidEllipse;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_columns;
    uint16_t    m_rows;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreatePrimitiveTileMap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_columns;
    uint16_t    m_rows;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreatePrimitiveTileArray;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
} VduOtfCreatePrimitiveSolidBitmap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreatePrimitiveMaskedBitmap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreatePrimitiveTransparentBitmap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords;
} VduOtfCreatePrimitiveGroup;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfSetPositionAndSliceSolidBitmap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfSetPositionAndSliceMaskedBitmap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfSetPositionAndSliceTransparentBitmap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfAdjustPositionAndSliceSolidBitmap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfAdjustPositionAndSliceMaskedBitmap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfAdjustPositionAndSliceTransparentBitmap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetSolidBitmapPixel;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetMaskedBitmapPixel;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetTransparentBitmapPixel;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetSolidBitmapPixels;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetMaskedBitmapPixels;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetTransparentBitmapPixels;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_col;
    uint16_t    m_row;
    uint16_t    m_img;
} VduOtfSetImageIdForTileInTileMap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_col;
    uint16_t    m_row;
    uint16_t    m_img;
} VduOtfSetImageIdForTileInTileArray;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_img;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetImagePixelInTileMap;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_img;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetImagePixelInTileArray;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveTriangleListOutline;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveSolidTriangleList;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_start[2];
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveTrianglFanOutline;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_start[2];
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveSolidTrianglFan;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_start[2];
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveTrianglStripOutline;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_start[2];
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveSolidTrianglStrip;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint8_t     m_color;
    VduCoords   m_coords[4];
} VduOtfCreatePrimitiveQuadOutline;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint8_t     m_color;
    VduCoords   m_coords[4];
} VduOtfCreatePrimitiveSolidQuad;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveQuadListOutline;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveSolidQuadList;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_start[2];
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveQuadStripOutline;

typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_start[2];
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveSolidQuadStrip;

#pragma pack(pop)
