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

#ifndef GDAL_VIEWER_BASE_RASTER_H
#define GDAL_VIEWER_BASE_RASTER_H

#include "HandmadeMath.h"
#include "sokol_gfx.h"

typedef struct {
    hmm_vec2 center;
    float zoom;

    sg_bindings bindings;
    sg_pipeline pipeline;
} _base_raster_t;

void _base_raster_init(void* self, int width, int height, const sg_shader_desc* shader_desc);

#endif /* GDAL_VIEWER_BASE_RASTER_H */

