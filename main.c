/*
 * Copyright 2021 Mattia Penati <mattia.penati@protonmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "HandmadeMath.h"
#include "stb_image.h"
#include "shaders/png_image.h"
#include <stdio.h>
#include <math.h>

typedef struct {
    float x, y, z;
    float u, v;
} vertex_t;

typedef struct {
    hmm_vec2 center;
    float zoom;

    hmm_mat4 proj;
} camera_t;

static void camera_init(camera_t* camera)
{
    camera->center = HMM_Vec2(0.0f, 0.0f);
    camera->zoom = 1.0f;
}

static void camera_zoom(camera_t* camera, float scroll)
{
    const float speed = 40.0f;

    float y = camera->zoom;
    float x = speed * (y - 1.0f / y);

    x += scroll;

    x /= 2.0f * speed;
    y = x + sqrtf(x * x + 1.0f);

    camera->zoom = y;
}

static void camera_move(camera_t* camera, float dx, float dy)
{
    camera->center.X += dx / camera->zoom;
    camera->center.Y += dy / camera->zoom;
}

static void camera_update(camera_t* camera, int width, int height)
{
    float half_width = 0.5f * width;
    float half_height = 0.5f * height;

    hmm_mat4 view = HMM_Orthographic(-half_width, half_width, -half_height, half_height, -1.0f, 1.0f);
    hmm_mat4 center = HMM_Translate(HMM_Vec3(camera->center.X, -camera->center.Y, 0.0f));
    hmm_mat4 zoom = HMM_Scale(HMM_Vec3(camera->zoom, camera->zoom, 1.0));

    camera->proj = HMM_MultiplyMat4(HMM_MultiplyMat4(view, zoom), center);
}

static struct {
    const char* filename;

    sg_bindings bindings;
    sg_pipeline pipeline;
    sg_pass_action pass_action;
    camera_t camera;
} state;

static void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext(),
    });

    int image_width, image_height, image_components;
    unsigned char* pixels = stbi_load(state.filename, &image_width, &image_height, &image_components, 4);
    state.bindings.fs_images[SLOT_tex] = sg_make_image(&(sg_image_desc){
        .width = image_width,
        .height = image_height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_NEAREST,
        .content.subimage[0][0] = {
            .ptr = pixels,
            .size = image_width * image_height * 4,
        },
    });
    stbi_image_free(pixels);

    vertex_t vertices[] = {
        {  image_width * 0.5f,  image_height * 0.5f, 0.0f, 1.0f, 0.0f },
        {  image_width * 0.5f, -image_height * 0.5f, 0.0f, 1.0f, 1.0f },
        { -image_width * 0.5f, -image_height * 0.5f, 0.0f, 0.0f, 1.0f },
        { -image_width * 0.5f,  image_height * 0.5f, 0.0f, 0.0f, 0.0f },
    };
    state.bindings.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .size = sizeof(vertices),
        .content = vertices,
        .label = "quad-vertices",
    });

    uint16_t indices[] = {
        0, 1, 3,
        1, 2, 3,
    };
    state.bindings.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .size = sizeof(indices),
        .content = indices,
        .label = "quad-indices",
    });

    state.pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = sg_make_shader(png_image_shader_desc()),
        .layout = {
            .attrs = {
                [ATTR_vs_position] = { .format = SG_VERTEXFORMAT_FLOAT3 },
                [ATTR_vs_texCoord] = { .format = SG_VERTEXFORMAT_FLOAT2 },
            },
        },
        .index_type = SG_INDEXTYPE_UINT16,
        .blend = {
            .enabled = true,
            .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
            .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
        },
        .label = "quad-pipeline",
    });

    state.pass_action = (sg_pass_action){
        .colors[0] = {
            .action = SG_ACTION_CLEAR,
            .val = { 0.5f, 0.5f, 0.5f, 1.0f },
        },
    };

    camera_init(&state.camera);
}

static void frame(void) {
    const int width = sapp_width();
    const int height = sapp_height();

    camera_update(&state.camera, width, height);

    vs_params_t vs_params = {
        .proj = state.camera.proj,
    };

    sg_begin_default_pass(&state.pass_action, width, height);
    sg_apply_pipeline(state.pipeline);
    sg_apply_bindings(&state.bindings);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
    sg_draw(0, 6, 1);
    sg_end_pass();
    sg_commit();
}

static void event(const sapp_event* event) {
    switch (event->type) {
    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        camera_zoom(&state.camera, event->scroll_y);
        break;
    case SAPP_EVENTTYPE_MOUSE_DOWN:
        if (event->mouse_button == SAPP_MOUSEBUTTON_LEFT)
            sapp_lock_mouse(true);
        break;
    case SAPP_EVENTTYPE_MOUSE_UP:
        if (event->mouse_button == SAPP_MOUSEBUTTON_LEFT)
            sapp_lock_mouse(false);
        break;
    case SAPP_EVENTTYPE_MOUSE_MOVE:
        if (sapp_mouse_locked())
            camera_move(&state.camera, event->mouse_dx, event->mouse_dy);
        break;
    case SAPP_EVENTTYPE_KEY_DOWN:
        if (event->key_code == SAPP_KEYCODE_ESCAPE)
            sapp_quit();
        break;
    case SAPP_EVENTTYPE_CHAR:
        if (event->char_code == '0')
            camera_init(&state.camera);
    default:
        break;
    }
}

static void cleanup(void) {
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char *argv[]) {
    if (argc != 2)
    {
        fprintf(stderr, "usage: imagevw <inputfile>\n");
        exit(1);
    }

    state.filename = argv[1];

    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .event_cb = event,
        .cleanup_cb = cleanup,
        .window_title = "Image viewer",
    };
}
