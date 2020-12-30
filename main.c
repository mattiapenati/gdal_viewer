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
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_time.h"
#include "cimgui.h"
#include "sokol_imgui.h"
#include "gdal.h"
#include <stdio.h>

static struct {
    const char* filename;
    void* raster;
    sg_pass_action pass_action;
    uint64_t last_time;
} state;

static void* gdal_load(const char* filename, int* width, int* height)
{
    const unsigned int open_flags= GDAL_OF_RASTER | GDAL_OF_READONLY;
    GDALDatasetH dataset = GDALOpenEx(filename, open_flags,  NULL, NULL, NULL);
    if (dataset == NULL)
    {
        fprintf(stderr, "unable to open %s\n", filename);
        exit(1);
    }

    GDALDriverH driver = GDALGetDatasetDriver(dataset);
    const char* driver_name = GDALGetDriverShortName(driver);
    if(strcmp("PNG", driver_name) == 0)
    {
        const int band_count = GDALGetRasterCount(dataset);
        if (band_count != 3 && band_count != 4)
        {
            fprintf(stderr, "unsupported format\n");
            goto error;
        }

        static const size_t ColorOffset[GCI_Max] = {
            [GCI_RedBand] = 0,
            [GCI_GreenBand] = 1,
            [GCI_BlueBand] = 2,
            [GCI_AlphaBand] = 3,
        };

        const int x_size = GDALGetRasterXSize(dataset);
        const int y_size = GDALGetRasterYSize(dataset);

        const size_t image_size = 4 * x_size * y_size;
        unsigned char* pixels = malloc(image_size);
        memset(pixels, ~(0x0), image_size);

        for (int band_index = 1; band_index <= band_count; ++band_index)
        {
            GDALRasterBandH band = GDALGetRasterBand(dataset, band_index);

            const GDALDataType band_data_type = GDALGetRasterDataType(band);
            if (band_data_type != GDT_Byte)
            {
                fprintf(stderr, "unsupported image data type\n");
                goto error;
            }

            const GDALColorInterp color_interp = GDALGetRasterColorInterpretation(band);
            const size_t offset = ColorOffset[color_interp];

            CPLErr error = GDALRasterIO(band, GF_Read, 0, 0, x_size, y_size,
                pixels + offset, x_size, y_size, band_data_type, 4, 4 * x_size);
            if (error == CE_Failure)
            {
                fprintf(stderr, "unable to read the file\n");
                goto error;
            }
        }

        *width = x_size;
        *height = y_size;
        return pixels;
    }
    else
    {
        fprintf(stderr, "unsupported format\n");
        goto error;
    }

    return NULL;

error:
    GDALClose(dataset);
    exit(1);
}

static void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext(),
    });

    stm_setup();
    simgui_setup(&(simgui_desc_t){});

    int width, height;
    void* pixels = gdal_load(state.filename, &width, &height);
    state.raster = make_rgba_raster(pixels, width, height, SG_PIXELFORMAT_RGBA8);
    free(pixels);

    state.pass_action = (sg_pass_action){
        .colors[0] = {
            .action = SG_ACTION_CLEAR,
            .val = { 0.5f, 0.5f, 0.5f, 1.0f },
        }
    };
}

static void frame(void) {
    const int width = sapp_width();
    const int height = sapp_height();
    const float delta_time = stm_sec(stm_laptime(&state.last_time));

    simgui_new_frame(width, height, delta_time);

    igBegin("Information", NULL, 0);
    igText("Filename: %s", state.filename);
    igEnd();

    sg_begin_default_pass(&state.pass_action, width, height);
    raster_draw(state.raster, width, height);
    simgui_render();
    sg_end_pass();
    sg_commit();
}

static void event(const sapp_event* event) {
    if(simgui_handle_event(event))
        return;

    switch (event->type) {
    case SAPP_EVENTTYPE_MOUSE_SCROLL:
        raster_zoom(state.raster, event->scroll_y);
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
            raster_move(state.raster, event->mouse_dx, event->mouse_dy);
        break;
    case SAPP_EVENTTYPE_KEY_DOWN:
        if (event->key_code == SAPP_KEYCODE_ESCAPE)
            sapp_quit();
        break;
    case SAPP_EVENTTYPE_CHAR:
        if (event->char_code == '0')
            raster_reset_view(state.raster);
    default:
        break;
    }
}

static void cleanup(void) {
    raster_destroy(state.raster);

    simgui_shutdown();
    sg_shutdown();
}

sapp_desc sokol_main(int argc, char *argv[]) {
    GDALAllRegister();

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
