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

extern void debug_log(const char* fmt, ...);

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
Pixel * frameBuffer;
PingoDepth * my_zetaBuffer;


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
  debug_log("@%i\n", __LINE__);
  frameBuffer = (Pixel*) heap_caps_malloc(sizeof(Pixel) * 160 * 120, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
  debug_log("@%i\n", __LINE__);
  my_zetaBuffer = (PingoDepth*) heap_caps_malloc(sizeof(PingoDepth) * 160 * 120, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
  debug_log("@%i\n", __LINE__);

  totalSize = size;
  backend->backend.init = &init;
  backend->backend.beforeRender = &beforeRender;
  backend->backend.afterRender = &afterRender;
  backend->backend.getFrameBuffer = &getFrameBuffer;
  backend->backend.getZetaBuffer = &getZetaBuffer;
  backend->backend.drawPixel = 0;
  debug_log("@%i\n", __LINE__);
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
  debug_log("@%i\n", __LINE__);
  Vec2i size = {width, height};

  debug_log("@%i\n", __LINE__);
  di_render_init(&my_backend, size);
  debug_log("@%i\n", __LINE__);
  Renderer renderer;
  debug_log("@%i\n", __LINE__);
  rendererInit(&renderer, size,(BackEnd*) &my_backend );
  debug_log("@%i\n", __LINE__);
  rendererSetCamera(&renderer,(Vec4i){0,0,size.x,size.y});
  debug_log("@%i\n", __LINE__);

  Scene s;
  debug_log("@%i\n", __LINE__);
  sceneInit(&s);
  debug_log("@%i\n", __LINE__);
  rendererSetScene(&renderer, &s);
  debug_log("@%i\n", __LINE__);

  Object object;
  debug_log("@%i %X %X\n", __LINE__, &mesh_teapot, object.mesh);
  object.mesh = &mesh_teapot;
  debug_log("@%i %X %X\n", __LINE__, &mesh_teapot, object.mesh);

/*  debug_log("@%i\n", __LINE__);
  Material m;
  debug_log("@%i\n", __LINE__);
  Texture tex;
  debug_log("@%i\n", __LINE__);
  Pixel solid[4];
  debug_log("@%i\n", __LINE__);
  memset(solid, 0xFF, sizeof(solid));
  debug_log("@%i\n", __LINE__);
  tex.frameBuffer = solid;
  debug_log("@%i\n", __LINE__);
  m.texture = &tex;
  debug_log("@%i\n", __LINE__);
  object.material = &m;
*/
  debug_log("@%i\n", __LINE__);

  sceneAddRenderable(&s, object_as_renderable(&object));
  debug_log("@%i\n", __LINE__);

  float phi = 0;
  Mat4 t;

  debug_log("@%i\n", __LINE__);
  // PROJECTION MATRIX - Defines the type of projection used
  renderer.camera_projection = mat4Perspective( 1, 2500.0,(float)size.x / (float)size.y, 0.6);
  debug_log("@%i\n", __LINE__);

  //VIEW MATRIX - Defines position and orientation of the "camera"
  Mat4 v = mat4Translate((Vec3f) { 0,2,-35});
  debug_log("@%i\n", __LINE__);

  Mat4 rotateDown = mat4RotateX(-0.40); //Rotate around origin/orbit
  debug_log("@%i\n", __LINE__);
  renderer.camera_view = mat4MultiplyM(&rotateDown, &v );
  debug_log("@%i\n", __LINE__);

  //TEA TRANSFORM - Defines position and orientation of the object
  debug_log("@%i\n", __LINE__);
  object.transform = mat4RotateZ(3.142128);
  debug_log("@%i\n", __LINE__);
  t = mat4RotateZ(0);
  debug_log("@%i\n", __LINE__);
  object.transform = mat4MultiplyM(&object.transform, &t );
  debug_log("@%i\n", __LINE__);

  //SCENE
  debug_log("@%i\n", __LINE__);
  s.transform = mat4RotateY(phi);
  debug_log("@%i\n", __LINE__);
  phi += 0.01;
  debug_log("@%i\n", __LINE__);

  rendererRender(&renderer);
  debug_log("@%i\n", __LINE__);
}

void cdebug_log(int line) {
  debug_log("@%i\n", line);
}

} // extern "C"

void DiRender::render() {
  do_render(m_width, m_height);
}
