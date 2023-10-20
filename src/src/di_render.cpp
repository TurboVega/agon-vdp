// di_render.cpp - Function definitions for drawing bitmaps via 3D rendering
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
// This code reliese on 'pingo' for its rendering functions. Refer to the
// README.md and LICENSE in the pingo directory for more information.
//

#include "di_render.h"
#include "esp_heap_caps.h"

extern "C" {

#include "pingo/assets/teapot.h"

#include "pingo/render/mesh.h"
#include "pingo/render/object.h"
#include "pingo/render/pixel.h"
#include "pingo/render/renderer.h"
#include "pingo/render/scene.h"
#include "pingo/render/backend.h"
#include "pingo/render/depth.h"
#include <cstring>

typedef struct Pixel Pixel;

typedef  struct {
    BackEnd backend;
    Vec2i size;
} DiRenderBackEnd;

void di_render_init(DiRenderBackEnd * backend, Vec2i size);

DiRenderBackEnd my_backend;
Vec4i rect;
Vec2i totalSize;
PingoDepth * zetaBuffer;
Pixel EXT_RAM_ATTR frameBuffer[160 * 120];
PingoDepth EXT_RAM_ATTR my_zetaBuffer[160 * 120];


void init( Renderer * ren, BackEnd * backEnd, Vec4i _rect) {
    rect = _rect;

}

void beforeRender( Renderer * ren, BackEnd * backEnd) {
    //DiRenderBackEnd * backend = (DiRenderBackEnd *) backEnd;
}

void afterRender( Renderer * ren,  BackEnd * backEnd) {
}

Pixel * getFrameBuffer( Renderer * ren,  BackEnd * backEnd) {
    return frameBuffer;
}

PingoDepth * getZetaBuffer( Renderer * ren,  BackEnd * backEnd) {
    return my_zetaBuffer;
}

void di_render_init( DiRenderBackEnd * backend, Vec2i size) {
    totalSize = size;
    backend->backend.init = &init;
    backend->backend.beforeRender = &beforeRender;
    backend->backend.afterRender = &afterRender;
    backend->backend.getFrameBuffer = &getFrameBuffer;
    backend->backend.getZetaBuffer = &getZetaBuffer;
    backend->backend.drawPixel = 0;
}

} // extern "C"

//-----------------------------------------

DiRender::DiRender(uint32_t width, uint32_t height, uint16_t flags) :
  DiBitmap(width, height, flags) {
}

DiRender::~DiRender() {
}

extern "C" {

void do_render(int width, int height) {
  Vec2i size = {width, height};

  di_render_init(&my_backend, size);
  Renderer renderer;
  rendererInit(&renderer, size,(BackEnd*) &my_backend );
  rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});

  Scene s;
  sceneInit(&s);
  rendererSetScene(&renderer, &s);

  Object object;
  object.mesh = &mesh_teapot;

  Material m;
  Texture tex;
  Pixel solid[4];
  memset(solid, 0xFF, sizeof(solid));
  tex.frameBuffer = solid;
  m.texture = &tex;
  object.material = &m;

  sceneAddRenderable(&s, object_as_renderable(&object));

  float phi = 0;
  Mat4 t;

  // PROJECTION MATRIX - Defines the type of projection used
  renderer.camera_projection = mat4Perspective( 1, 2500.0,(float)size.x / (float)size.y, 0.6);

  //VIEW MATRIX - Defines position and orientation of the "camera"
  Mat4 v = mat4Translate((Vec3f) { 0,2,-35});

  Mat4 rotateDown = mat4RotateX(-0.40); //Rotate around origin/orbit
  renderer.camera_view = mat4MultiplyM(&rotateDown, &v );

  //TEA TRANSFORM - Defines position and orientation of the object
  object.transform = mat4RotateZ(3.142128);
  t = mat4RotateZ(0);
  object.transform = mat4MultiplyM(&object.transform, &t );

  //SCENE
  s.transform = mat4RotateY(phi);
  phi += 0.01;

  rendererRender(&renderer);
}

} // extern "C"

void DiRender::render() {
  do_render(m_width, m_height);
}
