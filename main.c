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


static void init(void) {
    sg_setup(&(sg_desc){
        .context = sapp_sgcontext(),
    });

    stm_setup();
    simgui_setup(&(simgui_desc_t){});

    state.raster = make_gdal_raster(state.filename);
    if (state.raster == NULL)
        exit(1);

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
