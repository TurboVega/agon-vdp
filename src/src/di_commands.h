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

// VDU 23, 30, 0, id; flags; :  Set flags for primitive
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_flags;
} VduOtfSetFlagsForPrimitive;

// VDU 23, 30, 1, id; x; y; :  Set primitive position
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
} VduOtfSetPrimitivePosition;

// VDU 23, 30, 2, id; x; y; :  Adjust primitive position
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
} VduOtfAdjustPrimitivePosition;

// VDU 23, 30, 3, id; :  Delete primitive
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
} VduOtfDeletePrimitive;

// VDU 23, 30, 4, id; pid; flags; x; y; color :  Create primitive: Point
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfCreatePrimitivePoint;

// VDU 23, 30, 5, id; pid; flags; x1; y1; x2; y2; color :  Create primitive: Line
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords[2];
    uint8_t     m_color;
} VduOtfCreatePrimitiveLine;

// VDU 23, 30, 6, id; pid; flags; x1; y1; x2; y2; x3; y3; color :  Create primitive: Triangle Outline
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords[3];
    uint8_t     m_color;
} VduOtfCreatePrimitiveTriangleOutline;

// VDU 23, 30, 7, id; pid; flags; x1; y1; x2; y2; x3; y3; color :  Create primitive: Solid Triangle
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords[3];
    uint8_t     m_color;
} VduOtfCreatePrimitiveSolidTriangle;

// VDU 23, 30, 8, id; pid; flags; x; y; w; h; color :  Create primitive: Rectangle Outline
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

// VDU 23, 30, 9, id; pid; flags; x; y; w; h; color :  Create primitive: Solid Rectangle
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

// VDU 23, 30, 10, id; pid; flags; x; y; w; h; color :  Create primitive: Ellipse Outline
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

// VDU 23, 30, 11, id; pid; flags; x; y; w; h; color :  Create primitive: Solid Ellipse
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

// VDU 23, 30, 12, id; pid; flags; cols; rows; w; h; :  Create primitive: Tile Map
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

// VDU 23, 30, 13, id; pid; flags; cols; rows; w; h; :  Create primitive: Tile Array
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

// VDU 23, 30, 14, id; pid; flags; w; h; :  Create primitive: Solid Bitmap
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
} VduOtfCreatePrimitiveSolidBitmap;

// VDU 23, 30, 15, id; pid; flags; w; h; color :  Create primitive: Masked Bitmap
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreatePrimitiveMaskedBitmap;

// VDU 23, 30, 16, id; pid; flags; w; h; color :  Create primitive: Transparent Bitmap
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreatePrimitiveTransparentBitmap;

// VDU 23, 30, 17, id; pid; flags; x; y; :  Create primitive: Group
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords;
} VduOtfCreatePrimitiveGroup;

// VDU 23, 30, 18, id; x; y; s; h; :  Set position & slice solid bitmap
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfSetPositionAndSliceSolidBitmap;

// VDU 23, 30, 19, id; x; y; s; h; :  Set position & slice masked bitmap
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfSetPositionAndSliceMaskedBitmap;

// VDU 23, 30, 20, id; x; y; s; h; :  Set position & slice transparent bitmap
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfSetPositionAndSliceTransparentBitmap;

// VDU 23, 30, 21, id; x; y; s; h; :  Adjust position & slice solid bitmap
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfAdjustPositionAndSliceSolidBitmap;

// VDU 23, 30, 22, id; x; y; s; h; :  Adjust position & slice masked bitmap
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfAdjustPositionAndSliceMaskedBitmap;

// VDU 23, 30, 23, id; x; y; s; h; :  Adjust position & slice transparent bitmap
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_s;
    uint16_t    m_h;
} VduOtfAdjustPositionAndSliceTransparentBitmap;

// VDU 23, 30, 24, id; x; y; color :  Set solid bitmap pixel
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetSolidBitmapPixel;

// VDU 23, 30, 25, id; x; y; color :  Set masked bitmap pixel
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetMaskedBitmapPixel;

// VDU 23, 30, 26, id; x; y; color :  Set transparent bitmap pixel
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetTransparentBitmapPixel;

// VDU 23, 30, 27, id; x; y; n; c0, c1, c2, ... :  Set solid bitmap pixels
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetSolidBitmapPixels;

// VDU 23, 30, 28, id; x; y; n; c0, c1, c2, ... :  Set masked bitmap pixels
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetMaskedBitmapPixels;

// VDU 23, 30, 29, id; x; y; n; c0, c1, c2, ... :  Set transparent bitmap pixels
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetTransparentBitmapPixels;

// VDU 23, 30, 30, id; pid; flags; w; h; :  Create Solid Bitmap for Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
} VduOtfCreateSolidBitmapForTileMap;

// VDU 23, 30, 31, id; pid; flags; w; h; color :  Create Masked Bitmap for Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreateMaskedBitmapForTileMap;

// VDU 23, 30, 32, id; pid; flags; w; h; color :  Create Transparent Bitmap for Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreateTransparentBitmapForTileMap;

// VDU 23, 30, 33, id; pid; flags; w; h; :  Create Solid Bitmap for Tile Array
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
} VduOtfCreateSolidBitmapForTileArray;

// VDU 23, 30, 34, id; pid; flags; w; h; color :  Create Masked Bitmap for Tile Array
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreateMaskedBitmapForTileArray;

// VDU 23, 30, 35, id; pid; flags; w; h; color :  Create Transparent Bitmap for Tile Array
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_width;
    uint16_t    m_height;
    uint8_t     m_color;
} VduOtfCreateTransparentBitmapForTileArray;

// VDU 23, 30, 36, id; col; row; bmid; :  Set bitmap ID for tile in Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_col;
    uint16_t    m_row;
    uint16_t    m_bmid;
} VduOtfSetBitmapIdForTileInTileMap;

// VDU 23, 30, 37, id; col; row; bmid; :  Set bitmap ID for tile in Tile Array
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_col;
    uint16_t    m_row;
    uint16_t    m_bmid;
} VduOtfSetBitmapIdForTileInTileArray;

// VDU 23, 30, 38, id; bmid; x; y; color :  Set solid bitmap pixel in Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetSolidBitmapPixelInTileMap;

// VDU 23, 30, 39, id; bmid; x; y; color :  Set masked bitmap pixel in Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetMaskedBitmapPixelInTileMap;

// VDU 23, 30, 40, id; bmid; x; y; color :  Set transparent bitmap pixel in Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetTransparentBitmapPixelInTileMap;

// VDU 23, 30, 41, id; bmid; x; y; n; c0, c1, c2, ... :  Set solid bitmap pixels in Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetSolidBitmapPixelsInTileMap;

// VDU 23, 30, 42, id; bmid; x; y; n; c0, c1, c2, ... :  Set masked bitmap pixels in Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetMaskedBitmapPixelsInTileMap;

// VDU 23, 30, 43, id; bmid; x; y; n; c0, c1, c2, ... :  Set transparent bitmap pixels in Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetTransparentBitmapPixelsInTileMap;

// VDU 23, 30, 44, id; bmid; x; y; color :  Set solid bitmap pixel in Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetSolidBitmapPixelInTileMap;

// VDU 23, 30, 45, id; bmid; x; y; color :  Set masked bitmap pixel in Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetMaskedBitmapPixelInTileMap;

// VDU 23, 30, 46, id; bmid; x; y; color :  Set transparent bitmap pixel in Tile Map
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint8_t     m_color;
} VduOtfSetTransparentBitmapPixelInTileMap;

// VDU 23, 30, 47, id; bmid; x; y; n; c0, c1, c2, ... :  Set solid bitmap pixels in Tile Array
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetSolidBitmapPixelsInTileArray;

// VDU 23, 30, 48, id; bmid; x; y; n; c0, c1, c2, ... :  Set masked bitmap pixels in Tile Array
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetMaskedBitmapPixelsInTileArray;

// VDU 23, 30, 49, id; bmid; x; y; n; c0, c1, c2, ... :  Set transparent bitmap pixels in Tile Array
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_bmid;
    VduCoords   m_coords;
    uint16_t    m_n;
    uint8_t     m_color[1];
} VduOtfSetTransparentBitmapPixelsInTileArray;

// VDU 23, 30, 50, id; pid; flags; x; y; columns; rows; :  Create primitive: Terminal
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    VduCoords   m_coords;
    uint16_t    m_columns;
    uint16_t    m_rows;
 } VduOtfCreatePrimitiveTerminal;

// VDU 23, 30, 51, id; :  Select Active Terminal
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
 } VduOtfSetActiveTerminal;

// VDU 23, 30, 52, id; char, fgcolor, bgcolor :  Define Terminal Character
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint8_t     m_char;
    uint8_t     m_fgcolor;
    uint8_t     m_bgcolor;
 } VduOtfDefineTerminalCharacter;

// VDU 23, 30, 53, id; firstchar, lastchar, fgcolor, bgcolor :  Define Terminal Character Range
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint8_t     m_firstchar;
    uint8_t     m_lastchar;
    uint8_t     m_fgcolor;
    uint8_t     m_bgcolor;
 } VduOtfDefineTerminalCharacters;

// VDU 23, 30, 54, id; pid; flags; n; c, x1; y1; ... xn; yn; :  Create primitive: Triangle List Outline
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveTriangleListOutline;

// VDU 23, 30, 55, id; pid; flags; n; c, x1; y1; ... xn; yn; :  Create primitive: Solid Triangle List
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveSolidTriangleList;

// VDU 23, 30, 56, id; pid; flags; n; c, sx0; sy0; sx1; sy1; ... xn; yn; :  Create primitive: Triangle Fan Outline
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

// VDU 23, 30, 57, id; pid; flags; n; c, sx0; sy0; sx1; sy1; ... xn; yn; :  Create primitive: Solid Triangle Fan
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

// VDU 23, 30, 58, id; pid; flags; n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn; :  Create primitive: Triangle Strip Outline
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

// VDU 23, 30, 59, id; pid; flags; n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn; :  Create primitive: Solid Triangle Strip
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

// VDU 23, 30, 60, id; pid; flags; c, x1; y1; x2; y2; x3; y3; x4; y4; :  Create primitive: Quad Outline
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint8_t     m_color;
    VduCoords   m_coords[4];
} VduOtfCreatePrimitiveQuadOutline;

// VDU 23, 30, 61, id; pid; flags; c, x1; y1; x2; y2; x3; y3; x4; y4; :  Create primitive: Solid Quad
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint8_t     m_color;
    VduCoords   m_coords[4];
} VduOtfCreatePrimitiveSolidQuad;

// VDU 23, 30, 62, id; pid; flags; n; c, x1; y1; ... xn; yn; :  Create primitive: Quad List Outline
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveQuadListOutline;

// VDU 23, 30, 63, id; pid; flags; n; c, x1; y1; ... xn; yn; :  Create primitive: Solid Quad List
typedef struct {
    VduHeader   m_header;
    uint16_t    m_id;
    uint16_t    m_pid;
    uint16_t    m_flags;
    uint16_t    m_n;
    uint8_t     m_color;
    VduCoords   m_coords[1];
} VduOtfCreatePrimitiveSolidQuadList;

// VDU 23, 30, 64, id; pid; flags; n; c, sx0; sy0; sx1; sy1; x1; y1; ... xn; yn; :  Create primitive: Quad Strip Outline
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

// VDU 23, 30, 65, id; pid; flags; n; c, x1; y1; ... xn; yn; :  Create primitive: Solid Quad Strip
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
