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

#include "HandmadeMath.h"
#include "sokol_gfx.h"

typedef struct {
    hmm_vec2 center;
    float zoom;

    sg_bindings bindings;
    sg_pipeline pipeline;
} image_t;

void image_init(image_t* image, void* pixels, int width, int height);
void image_reset(image_t* image);
void image_destroy(image_t* image);

void image_zoom(image_t* image, float scroll);
void image_move(image_t* image, float dx, float dy);

void image_draw(image_t* image, int app_width, int app_height);
