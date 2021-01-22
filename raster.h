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

#ifndef GDAL_VIEWER_RASTER_H
#define GDAL_VIEWER_RASTER_H

#include "sokol_gfx.h"

void* make_rgba_raster(void* pixels, int width, int height, sg_pixel_format pixel_format);
void* make_gdal_raster(const char* filename);

void raster_destroy(void* self);
void raster_move(void* self, float dx, float dy);
void raster_zoom(void* self, float scroll);
void raster_reset_view(void* self);
void raster_draw(void* self, int app_width, int app_height);

#endif /* GDAL_VIEWER_RASTER_H */
