/*
 * Copyright (c) 2023-2025 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include <zmk/endpoints.h>

#define NVC_PROFILE_COUNT 5

/* Portrait layout of the 160x68 panel (top to bottom):
 *   top band    24 px  battery + connection icon   (68x68 canvas, rows 44..67 visible)
 *   art        112 px  lv_image, pre-rotated by tools/png2lvgl.py
 *   bottom band 24 px  profile dots + layer name   (68x68 canvas, rows 0..23 visible)
 * Canvases are drawn in portrait orientation and rotated. */
#define CANVAS_SIZE 68
#define BAND 24
#define TOP_BAND_Y (CANVAS_SIZE - BAND)
#define ART_W 68  /* portrait */
#define ART_H 112
#define CANVAS_COLOR_FORMAT LV_COLOR_FORMAT_L8 // smallest type supported by sw_rotate
#define CANVAS_BUF_SIZE                                                                            \
    LV_CANVAS_BUF_SIZE(CANVAS_SIZE, CANVAS_SIZE, LV_COLOR_FORMAT_GET_BPP(CANVAS_COLOR_FORMAT),     \
                       LV_DRAW_BUF_STRIDE_ALIGN)

#define LVGL_BACKGROUND                                                                            \
    (IS_ENABLED(CONFIG_NICE_VIEW_CUSTOM_INVERTED) ? lv_color_black() : lv_color_white())
#define LVGL_FOREGROUND                                                                            \
    (IS_ENABLED(CONFIG_NICE_VIEW_CUSTOM_INVERTED) ? lv_color_white() : lv_color_black())

struct status_state {
    uint8_t battery;
    bool charging;
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
    bool profiles_connected[NVC_PROFILE_COUNT];
    bool profiles_bonded[NVC_PROFILE_COUNT];
    uint8_t layer_index;
    const char *layer_label;
#else
    bool connected;
#endif
};

struct battery_status_state {
    uint8_t level;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    bool usb_present;
#endif
};

void rotate_canvas(lv_obj_t *canvas);
void init_layout(lv_obj_t *obj, uint8_t *top, uint8_t *bottom);

/* Battery icon, 24x12 (bolt overlay extends 3 px above and below while charging) */
void draw_battery_icon(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, uint8_t level,
                       bool charging);

void init_label_dsc(lv_draw_label_dsc_t *label_dsc, lv_color_t color, const lv_font_t *font,
                    lv_text_align_t align);
void init_rect_dsc(lv_draw_rect_dsc_t *rect_dsc, lv_color_t bg_color);

void canvas_draw_rect(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                      lv_draw_rect_dsc_t *draw_dsc);
void canvas_draw_text(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, lv_coord_t max_w,
                      lv_draw_label_dsc_t *draw_dsc, const char *txt);
void canvas_draw_img(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, const lv_image_dsc_t *src,
                     lv_draw_image_dsc_t *draw_dsc);
