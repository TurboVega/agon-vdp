#include <string.h>
#include <stdio.h>
#include "renderer.h"
#include "sprite.h"
#include "pixel.h"
#include "depth.h"
#include "backend.h"
#include "scene.h"
#include "rasterizer.h"
#include "object.h"
/*#include "../backend/ttgobackend.h"*/


int renderFrame(Renderer * r, Renderable ren) {
    Texture * f = ren.impl;
    return rasterizer_draw_pixel_perfect((Vec2i) { 0, 0 }, r, f);
};

int renderSprite(Mat4 transform, Renderer * r, Renderable ren) {
    Sprite * s = ren.impl;
    Mat4 backUp = s->t;

    //Apply parent transform to the local transform
    s->t = mat4MultiplyM( & s->t, & transform);

    //Apply camera translation
    Mat4 newMat = mat4Translate((Vec3f) { -r->camera.x, -r->camera.y, 0 });
    s->t = mat4MultiplyM( & s->t, & newMat);

    /*
  if (mat4IsOnlyTranslation(&s->t)) {
      Vec2i off = {s->t.elements[2], s->t.elements[5]};
      rasterizer_draw_pixel_perfect(off,r, &s->frame);
      s->t = backUp;
      return 0;
  }

  if (mat4IsOnlyTranslationDoubled(&s->t)) {
      Vec2i off = {s->t.elements[2], s->t.elements[5]};
      rasterizer_draw_pixel_perfect_doubled(off,r, &s->frame);
      s->t = backUp;
      return 0;
  }*/

    rasterizer_draw_transformed(s->t, r, & s->frame);
    s->t = backUp;
    return 0;
};

//extern void cdebug_log(int line);

void renderRenderable(Mat4 transform, Renderer * r, Renderable ren) {
  //cdebug_log(__LINE__);
    renderingFunctions[ren.renderableType](transform, r, ren);
  //cdebug_log(__LINE__);
};

int renderScene(Mat4 transform, Renderer * r, Renderable ren) {
  //cdebug_log(__LINE__);
    Scene * s = ren.impl;
  //cdebug_log(__LINE__);
    if (!s->visible)
        return 0;

    //Apply hierarchy transfom
  //cdebug_log(__LINE__);
    Mat4 newTransform = mat4MultiplyM( & s->transform, & transform);
  //cdebug_log(__LINE__);
    for (int i = 0; i < s->numberOfRenderables; i++) {
  //cdebug_log(__LINE__);
        renderRenderable(newTransform, r, s->renderables[i]);
  //cdebug_log(__LINE__);
    }
  //cdebug_log(__LINE__);
    return 0;
};

#define MIN(a, b)(((a) < (b)) ? (a) : (b))
#define MAX(a, b)(((a) > (b)) ? (a) : (b))

int edgeFunction(const Vec2f * a, const Vec2f * b, const Vec2f * c) {
    return (c->x - a->x) * (b->y - a->y) - (c->y - a->y) * (b->x - a->x);
}

float isClockWise(float x1, float y1, float x2, float y2, float x3, float y3) {
    return (y2 - y1) * (x3 - x2) - (y3 - y2) * (x2 - x1);
}

int orient2d( Vec2i a,  Vec2i b,  Vec2i c)
{
    return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
}

void backendDrawPixel (Renderer * r, Texture * f, Vec2i pos, Pixel color, float illumination) {
    //If backend spcifies something..
    if (r->backEnd->drawPixel != 0)
        r->backEnd->drawPixel(f, pos, color, illumination);

    //By default call this
    texture_draw(f, pos, pixelMul(color,illumination));
}

int renderObject(Mat4 object_transform, Renderer * r, Renderable ren) {
  //cdebug_log(__LINE__);

    const Vec2i scrSize = r->frameBuffer.size;
  //cdebug_log(__LINE__);
    Object * o = ren.impl;
  //cdebug_log(__LINE__);

    // MODEL MATRIX
    Mat4 m = mat4MultiplyM( &o->transform, &object_transform  );
  //cdebug_log(__LINE__);

    // VIEW MATRIX
  //cdebug_log(__LINE__);
    Mat4 v = r->camera_view;
  //cdebug_log(__LINE__);
    Mat4 p = r->camera_projection;
  //cdebug_log(__LINE__);

    for (int i = 0; i < o->mesh->indexes_count; i += 3) {
  //cdebug_log(__LINE__);
        Vec3f * ver1 = &o->mesh->positions[o->mesh->pos_indices[i+0]];
  //cdebug_log(__LINE__);
        Vec3f * ver2 = &o->mesh->positions[o->mesh->pos_indices[i+1]];
  //cdebug_log(__LINE__);
        Vec3f * ver3 = &o->mesh->positions[o->mesh->pos_indices[i+2]];
  //cdebug_log(__LINE__);

        Vec2f tca = {0,0};
  //cdebug_log(__LINE__);
        Vec2f tcb = {0,0};
  //cdebug_log(__LINE__);
        Vec2f tcc = {0,0};
  //cdebug_log(__LINE__);

        if (o->material != 0) {
  //cdebug_log(__LINE__);
            tca = o->mesh->textCoord[o->mesh->tex_indices[i+0]];
  //cdebug_log(__LINE__);
            tcb = o->mesh->textCoord[o->mesh->tex_indices[i+1]];
  //cdebug_log(__LINE__);
            tcc = o->mesh->textCoord[o->mesh->tex_indices[i+2]];
  //cdebug_log(__LINE__);
        }
  //cdebug_log(__LINE__);

        Vec4f a =  { ver1->x, ver1->y, ver1->z, 1 };
  //cdebug_log(__LINE__);
        Vec4f b =  { ver2->x, ver2->y, ver2->z, 1 };
  //cdebug_log(__LINE__);
        Vec4f c =  { ver3->x, ver3->y, ver3->z, 1 };
  //cdebug_log(__LINE__);

        a = mat4MultiplyVec4( &a, &m);
  //cdebug_log(__LINE__);
        b = mat4MultiplyVec4( &b, &m);
  //cdebug_log(__LINE__);
        c = mat4MultiplyVec4( &c, &m);
  //cdebug_log(__LINE__);

        //Calc Face Normal
        Vec3f na = vec3fsubV(*((Vec3f*)(&a)), *((Vec3f*)(&b)));
  //cdebug_log(__LINE__);
        Vec3f nb = vec3fsubV(*((Vec3f*)(&a)), *((Vec3f*)(&c)));
  //cdebug_log(__LINE__);
        Vec3f normal = vec3Normalize(vec3Cross(na, nb));
  //cdebug_log(__LINE__);
        Vec3f light = vec3Normalize((Vec3f){-8,5,5});
  //cdebug_log(__LINE__);
        float diffuseLight = (1.0 + vec3Dot(normal, light)) *0.5;
  //cdebug_log(__LINE__);
        diffuseLight = MIN(1.0, MAX(diffuseLight, 0));
  //cdebug_log(__LINE__);

        a = mat4MultiplyVec4( &a, &v);
  //cdebug_log(__LINE__);
        b = mat4MultiplyVec4( &b, &v);
  //cdebug_log(__LINE__);
        c = mat4MultiplyVec4( &c, &v);
  //cdebug_log(__LINE__);

        a = mat4MultiplyVec4( &a, &p);
  //cdebug_log(__LINE__);
        b = mat4MultiplyVec4( &b, &p);
  //cdebug_log(__LINE__);
        c = mat4MultiplyVec4( &c, &p);
  //cdebug_log(__LINE__);


        //Triangle is completely behind camera
        if (a.z > 0 && b.z > 0 && c.z > 0)
           continue;
  //cdebug_log(__LINE__);

        // convert to device coordinates by perspective division
        a.w = 1.0 / a.w;
  //cdebug_log(__LINE__);
        b.w = 1.0 / b.w;
  //cdebug_log(__LINE__);
        c.w = 1.0 / c.w;
  //cdebug_log(__LINE__);
        a.x *= a.w; a.y *= a.w; a.z *= a.w;
  //cdebug_log(__LINE__);
        b.x *= b.w; b.y *= b.w; b.z *= b.w;
  //cdebug_log(__LINE__);
        c.x *= c.w; c.y *= c.w; c.z *= c.w;
  //cdebug_log(__LINE__);

        float clocking = isClockWise(a.x, a.y, b.x, b.y, c.x, c.y);
  //cdebug_log(__LINE__);
        if (clocking >= 0)
            continue;
  //cdebug_log(__LINE__);

        //Compute Screen coordinates
        float halfX = scrSize.x/2;
  //cdebug_log(__LINE__);
        float halfY = scrSize.y/2;
  //cdebug_log(__LINE__);
        Vec2i a_s = {a.x * halfX + halfX,  a.y * halfY + halfY};
  //cdebug_log(__LINE__);
        Vec2i b_s = {b.x * halfX + halfX,  b.y * halfY + halfY};
  //cdebug_log(__LINE__);
        Vec2i c_s = {c.x * halfX + halfX,  c.y * halfY + halfY};
  //cdebug_log(__LINE__);

        int32_t minX = MIN(MIN(a_s.x, b_s.x), c_s.x);
  //cdebug_log(__LINE__);
        int32_t minY = MIN(MIN(a_s.y, b_s.y), c_s.y);
  //cdebug_log(__LINE__);
        int32_t maxX = MAX(MAX(a_s.x, b_s.x), c_s.x);
  //cdebug_log(__LINE__);
        int32_t maxY = MAX(MAX(a_s.y, b_s.y), c_s.y);
  //cdebug_log(__LINE__);

        minX = MIN(MAX(minX, 0), r->frameBuffer.size.x);
  //cdebug_log(__LINE__);
        minY = MIN(MAX(minY, 0), r->frameBuffer.size.y);
  //cdebug_log(__LINE__);
        maxX = MIN(MAX(maxX, 0), r->frameBuffer.size.x);
  //cdebug_log(__LINE__);
        maxY = MIN(MAX(maxY, 0), r->frameBuffer.size.y);
  //cdebug_log(__LINE__);

        // Barycentric coordinates at minX/minY corner
        Vec2i minTriangle = { minX, minY };
  //cdebug_log(__LINE__);

        int32_t area =  orient2d( a_s, b_s, c_s);
  //cdebug_log(__LINE__);
        if (area == 0)
            continue;
  //cdebug_log(__LINE__);
        float areaInverse = 1.0/area;
  //cdebug_log(__LINE__);

        int32_t A01 = ( a_s.y - b_s.y); //Barycentric coordinates steps
  //cdebug_log(__LINE__);
        int32_t B01 = ( b_s.x - a_s.x); //Barycentric coordinates steps
  //cdebug_log(__LINE__);
        int32_t A12 = ( b_s.y - c_s.y); //Barycentric coordinates steps
  //cdebug_log(__LINE__);
        int32_t B12 = ( c_s.x - b_s.x); //Barycentric coordinates steps
  //cdebug_log(__LINE__);
        int32_t A20 = ( c_s.y - a_s.y); //Barycentric coordinates steps
  //cdebug_log(__LINE__);
        int32_t B20 = ( a_s.x - c_s.x); //Barycentric coordinates steps
  //cdebug_log(__LINE__);

        int32_t w0_row = orient2d( b_s, c_s, minTriangle);
  //cdebug_log(__LINE__);
        int32_t w1_row = orient2d( c_s, a_s, minTriangle);
  //cdebug_log(__LINE__);
        int32_t w2_row = orient2d( a_s, b_s, minTriangle);
  //cdebug_log(__LINE__);

        if (o->material != 0) {
  //cdebug_log(__LINE__);
            tca.x /= a.z;
  //cdebug_log(__LINE__);
            tca.y /= a.z;
  //cdebug_log(__LINE__);
            tcb.x /= b.z;
  //cdebug_log(__LINE__);
            tcb.y /= b.z;
  //cdebug_log(__LINE__);
            tcc.x /= c.z;
  //cdebug_log(__LINE__);
            tcc.y /= c.z;
  //cdebug_log(__LINE__);
        }

        for (int16_t y = minY; y < maxY; y++, w0_row += B12,w1_row += B20,w2_row += B01) {
  //cdebug_log(__LINE__);
            int32_t w0 = w0_row;
  //cdebug_log(__LINE__);
            int32_t w1 = w1_row;
  //cdebug_log(__LINE__);
            int32_t w2 = w2_row;
  //cdebug_log(__LINE__);

            for (int32_t x = minX; x < maxX; x++, w0 += A12, w1 += A20, w2 += A01) {
  //cdebug_log(__LINE__);

                if ((w0 | w1 | w2) < 0)
                    continue;
  //cdebug_log(__LINE__);

                float depth =  -( w0 * a.z + w1 * b.z + w2 * c.z ) * areaInverse;
  //cdebug_log(__LINE__);
                if (depth < 0.0 || depth > 1.0)
                    continue;
  //cdebug_log(__LINE__);

                if (depth_check(r->backEnd->getZetaBuffer(r,r->backEnd), x + y * scrSize.x, 1-depth ))
                    continue;
  //cdebug_log(__LINE__);

                depth_write(r->backEnd->getZetaBuffer(r,r->backEnd), x + y * scrSize.x, 1- depth );
  //cdebug_log(__LINE__);

                if (o->material != 0) {
                    //Texture lookup
  //cdebug_log(__LINE__);

                    float textCoordx = -(w0 * tca.x + w1 * tcb.x + w2 * tcc.x)* areaInverse * depth;
  //cdebug_log(__LINE__);
                    float textCoordy = -(w0 * tca.y + w1 * tcb.y + w2 * tcc.y)* areaInverse * depth;
  //cdebug_log(__LINE__);

                    Pixel text = texture_readF(o->material->texture, (Vec2f){textCoordx,textCoordy});
  //cdebug_log(__LINE__);
                    backendDrawPixel(r, &r->frameBuffer, (Vec2i){x,y}, text, diffuseLight);
  //cdebug_log(__LINE__);
                } else {
  //cdebug_log(__LINE__);
                    backendDrawPixel(r, &r->frameBuffer, (Vec2i){x,y}, pixelFromUInt8(255), diffuseLight);
  //cdebug_log(__LINE__);
                }
  //cdebug_log(__LINE__);

            }
  //cdebug_log(__LINE__);

        }
  //cdebug_log(__LINE__);
    }
  //cdebug_log(__LINE__);

    return 0;
};

int rendererInit(Renderer * r, Vec2i size, BackEnd * backEnd) {
    renderingFunctions[RENDERABLE_SPRITE] = & renderSprite;
    renderingFunctions[RENDERABLE_SCENE] = & renderScene;
    renderingFunctions[RENDERABLE_OBJECT] = & renderObject;

    r->scene = 0;
    r->clear = 1;
    r->clearColor = PIXELBLACK;
    r->backEnd = backEnd;

    r->backEnd->init(r, r->backEnd, (Vec4i) { 0, 0, 0, 0 });

    int e = 0;
    e = texture_init( & (r->frameBuffer), size, backEnd->getFrameBuffer(r, backEnd));
    if (e) return e;

    return 0;
}

//extern void cdebug_log(int line);


int rendererRender(Renderer * r) {
  //cdebug_log(__LINE__);

    int pixels = r->frameBuffer.size.x * r->frameBuffer.size.y;
  //cdebug_log(__LINE__);
    memset(r->backEnd->getZetaBuffer(r,r->backEnd), 0, pixels * sizeof (PingoDepth));
  //cdebug_log(__LINE__);

    r->backEnd->beforeRender(r, r->backEnd);
  //cdebug_log(__LINE__);

    //get current framebuffe from backend
    r->frameBuffer.frameBuffer = r->backEnd->getFrameBuffer(r, r->backEnd);
  //cdebug_log(__LINE__);

    //Clear draw buffer before rendering
    if (r->clear) {
  //cdebug_log(__LINE__);
        memset(r->backEnd->getFrameBuffer(r,r->backEnd), 0, pixels * sizeof (Pixel));
  //cdebug_log(__LINE__);
    }

  //cdebug_log(__LINE__);
    renderScene(mat4Identity(), r, sceneAsRenderable(r->scene));
  //cdebug_log(__LINE__);

    r->backEnd->afterRender(r, r->backEnd);
  //cdebug_log(__LINE__);

    return 0;
}

int rendererSetScene(Renderer * r, Scene * s) {
    if (s == 0)
        return 1; //nullptr scene

    r->scene = s;
    return 0;
}

int rendererSetCamera(Renderer * r, Vec4i rect) {
    r->camera = rect;
    r->backEnd->init(r, r->backEnd, rect);
    r->frameBuffer.size = (Vec2i) {
            rect.z, rect.w
};
    return 0;
}
