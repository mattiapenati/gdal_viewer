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
#include "shaders/rgba_shader.h"

typedef struct {
    _base_raster_t base;
} rgba_raster_t;

void* make_rgba_raster(void* pixels,
                       const int width,
                       const int height,
                       const sg_pixel_format pixel_format)
{
    static const size_t PixelSize[_SG_PIXELFORMAT_NUM] = {
        [SG_PIXELFORMAT_RGBA8] = 4,
        [SG_PIXELFORMAT_RGBA16] = 8,
    };

    rgba_raster_t* self = calloc(1, sizeof(*self));
    if (self == NULL)
        goto error;

    _base_raster_t* base = &self->base;
    _base_raster_init(base, width, height, rgba_shader_desc());

    const size_t pixel_size = PixelSize[pixel_format];
    if (pixel_size == 0)
        goto error;

    base->bindings.fs_images[0] = sg_make_image(&(sg_image_desc){
        .width = width,
        .height = height,
        .pixel_format = pixel_format,
        .min_filter = SG_FILTER_LINEAR,
        .mag_filter = SG_FILTER_NEAREST,
        .content.subimage[0][0] = {
            .ptr = pixels,
            .size = width * height * pixel_size,
        },
    });
    return self;

error:
    free(self);
    return NULL;
}
