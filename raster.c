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

#include "raster.h"
#include "base_raster.h"

typedef struct {
    hmm_mat4 proj;
} vs_params_t;

typedef struct {
    hmm_vec3 position;
    hmm_vec2 texCoord;
} vertex_t;

void _base_raster_init(void* self, int width, int height, const sg_shader_desc* shader_desc)
{
    _base_raster_t* raster = self;

    raster_reset_view(raster);

    const vertex_t vertices[] = {
        { {  width * 0.5f,  height * 0.5f, 0.0f }, { 1.0f, 0.0f } },
        { {  width * 0.5f, -height * 0.5f, 0.0f }, { 1.0f, 1.0f } },
        { { -width * 0.5f, -height * 0.5f, 0.0f }, { 0.0f, 1.0f } },
        { { -width * 0.5f,  height * 0.5f, 0.0f }, { 0.0f, 0.0f } },
    };
    raster->bindings.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_VERTEXBUFFER,
        .size = sizeof(vertices),
        .content = vertices,
    });

    const uint16_t indices[] = {
        0, 1, 3,
        1, 2, 3,
    };
    raster->bindings.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .type = SG_BUFFERTYPE_INDEXBUFFER,
        .size = sizeof(indices),
        .content = indices,
    });

    raster->pipeline = sg_make_pipeline(&(sg_pipeline_desc){
        .shader = sg_make_shader(shader_desc),
        .layout = {
            .attrs = {
                [0] = { .format = SG_VERTEXFORMAT_FLOAT3 },
                [1] = { .format = SG_VERTEXFORMAT_FLOAT2 },
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

void raster_destroy(void* self)
{
    free(self);
}

void raster_move(void* self, float dx, float dy)
{
    _base_raster_t* raster = self;

    raster->center.X += dx / raster->zoom;
    raster->center.Y += dy / raster->zoom;
}

void raster_zoom(void* self, float scroll)
{
    _base_raster_t* raster = self;

    const float scroll_speed = 40.0f;

    float y = raster->zoom;
    float x = scroll_speed * (y - 1.0f / y);

    x += scroll;

    x /= 2.0f * scroll_speed;
    y = x + sqrtf(x * x + 1.0f);

    raster->zoom = y;
}

void raster_reset_view(void* self)
{
    _base_raster_t* raster = self;

    raster->center = HMM_Vec2(0.0f, 0.0f);
    raster->zoom = 1.0f;
}

void raster_draw(void* self, int app_width, int app_height)
{
    _base_raster_t* raster = self;

    const hmm_mat4 view = HMM_Orthographic(
            -0.5f * app_width, 0.5f * app_width,
            -0.5f * app_height, 0.5f * app_height,
            -1.0f, 1.0f);
    const hmm_mat4 center = HMM_Translate(
            HMM_Vec3(raster->center.X, -raster->center.Y, 0.0f));
    const hmm_mat4 zoom = HMM_Scale(
            HMM_Vec3(raster->zoom, raster->zoom, 1.0));

    const vs_params_t vs_params = {
        .proj = HMM_MultiplyMat4(HMM_MultiplyMat4(view, zoom), center),
    };

    sg_apply_pipeline(raster->pipeline);
    sg_apply_bindings(&raster->bindings);
    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));
    sg_draw(0, 6, 1);
}
