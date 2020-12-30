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

#include "image.h"
#include "shaders/image_shader.h"
#include <math.h>

typedef struct {
    hmm_vec3 position;
    hmm_vec2 texCoord;
} vertex_t;

void image_init(image_t* image, void* pixels, const int width, const int height) {
    image->center = HMM_Vec2(0.0f, 0.0f);
    image->zoom = 1.0f;

    const vertex_t vertices[] = {
        { {  width * 0.5f,  height * 0.5f, 0.0f }, { 1.0f, 0.0f } },
        { {  width * 0.5f, -height * 0.5f, 0.0f }, { 1.0f, 1.0f } },
        { { -width * 0.5f, -height * 0.5f, 0.0f }, { 0.0f, 1.0f } },
        { { -width * 0.5f,  height * 0.5f, 0.0f }, { 0.0f, 0.0f } },
    };
    image->bindings.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .size = sizeof(vertices),
        .content = vertices,
    });

    const uint16_t indices[] = {
        0, 1, 3,
        1, 2, 3,
    };
    image->bindings.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .size = sizeof(indices),
        .content = indices,
    });

    image->bindings.fs_images[SLOT_tex] = sg_make_image(&(sg_image_desc){
        .width = width,
        .height = height,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_NEAREST,
        .content.subimage[0][0] = {
            .ptr = pixels,
            .size = width * height * 4,
        },
    });

    image->pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = sg_make_shader(image_shader_desc()),
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
    });
}

void image_reset(image_t* image) {
    image->center = HMM_Vec2(0.0f, 0.0f);
    image->zoom = 1.0f;
}

void image_destroy(image_t* image) {
}

void image_zoom(image_t* image, const float scroll) {
    const float scroll_speed = 40.0f;

    float y = image->zoom;
    float x = scroll_speed * (y - 1.0f / y);

    x += scroll;

    x /= 2.0f * scroll_speed;
    y = x + sqrtf(x * x + 1.0f);

    image->zoom = y;
}

void image_move(image_t* image, const float dx, float dy) {
    image->center.X += dx / image->zoom;
    image->center.Y += dy / image->zoom;
}

void image_draw(image_t* image, const int app_width, const int app_height) {
    /* update projection matrix */
    const hmm_mat4 view = HMM_Orthographic(
            -0.5f * app_width, 0.5f * app_width,
            -0.5f * app_height, 0.5f * app_height,
            -1.0f, 1.0f);
    const hmm_mat4 center = HMM_Translate(
            HMM_Vec3(image->center.X, -image->center.Y, 0.0f));
    const hmm_mat4 zoom = HMM_Scale(
            HMM_Vec3(image->zoom, image->zoom, 1.0));

    const vs_params_t vs_params = {
        .proj = HMM_MultiplyMat4(HMM_MultiplyMat4(view, zoom), center),
    };

    sg_apply_pipeline(image->pipeline);
    sg_apply_bindings(&image->bindings);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, &vs_params, sizeof(vs_params));
    sg_draw(0, 6, 1);
}

