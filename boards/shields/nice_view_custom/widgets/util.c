/*
 * Copyright (c) 2023-2025 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include "util.h"

LV_IMG_DECLARE(bolt);
LV_IMG_DECLARE(nice_view_custom_art);

void rotate_canvas(lv_obj_t *canvas) {
    uint8_t *buf = lv_canvas_get_draw_buf(canvas)->data;
    static uint8_t buf_copy[CANVAS_BUF_SIZE];
    memcpy(buf_copy, buf, sizeof(buf_copy));

    const uint32_t stride = lv_draw_buf_width_to_stride(CANVAS_SIZE, CANVAS_COLOR_FORMAT);
    lv_draw_sw_rotate(buf_copy, buf, CANVAS_SIZE, CANVAS_SIZE, stride, stride,
                      LV_DISPLAY_ROTATION_270, CANVAS_COLOR_FORMAT);
}

void init_canvases(lv_obj_t *obj, uint8_t *top, uint8_t *middle, uint8_t *bottom) {
    lv_obj_set_size(obj, 160, 68);

    lv_obj_t *c_top = lv_canvas_create(obj);
    lv_obj_align(c_top, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_canvas_set_buffer(c_top, top, CANVAS_SIZE, CANVAS_SIZE, CANVAS_COLOR_FORMAT);

    lv_obj_t *c_middle = lv_canvas_create(obj);
    lv_obj_align(c_middle, LV_ALIGN_TOP_LEFT, 24, 0);
    lv_canvas_set_buffer(c_middle, middle, CANVAS_SIZE, CANVAS_SIZE, CANVAS_COLOR_FORMAT);

    lv_obj_t *c_bottom = lv_canvas_create(obj);
    lv_obj_align(c_bottom, LV_ALIGN_TOP_LEFT, -44, 0);
    lv_canvas_set_buffer(c_bottom, bottom, CANVAS_SIZE, CANVAS_SIZE, CANVAS_COLOR_FORMAT);
}

/* One battery row, 14 px tall: "[=====]   87%" */
void draw_battery_row(lv_obj_t *canvas, lv_coord_t y, uint8_t level, bool charging) {
    lv_draw_rect_dsc_t bg, fg;
    init_rect_dsc(&bg, LVGL_BACKGROUND);
    init_rect_dsc(&fg, LVGL_FOREGROUND);
    lv_draw_label_dsc_t pct_dsc;
    init_label_dsc(&pct_dsc, LVGL_FOREGROUND, &lv_font_unscii_8, LV_TEXT_ALIGN_RIGHT);

    level = MIN(level, 100);

    /* outline 22x12 + nub, leaves x 32..67 for "100%" */
    const lv_coord_t x = 2;
    canvas_draw_rect(canvas, x, y + 1, 22, 12, &fg);
    canvas_draw_rect(canvas, x + 1, y + 2, 20, 10, &bg);
    canvas_draw_rect(canvas, x + 22, y + 4, 2, 6, &fg);

    if (charging) {
        /* Empty outline + bolt; the percentage text still shows the level */
        lv_draw_image_dsc_t img_dsc;
        lv_draw_image_dsc_init(&img_dsc);
        canvas_draw_img(canvas, x + 6, y - 2, &bolt, &img_dsc);
    } else {
        int fill = (level * 18 + 50) / 100;
        if (fill > 0) {
            canvas_draw_rect(canvas, x + 2, y + 3, fill, 8, &fg);
        }
    }

    char pct[6];
    snprintf(pct, sizeof(pct), "%u%%", level);
    canvas_draw_text(canvas, 32, y + 3, CANVAS_SIZE - 32, &pct_dsc, pct);
}

void draw_art(lv_obj_t *canvas) {
    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    const lv_image_dsc_t *art = &nice_view_custom_art;
    lv_coord_t x = (CANVAS_SIZE - (lv_coord_t)art->header.w) / 2;
    lv_coord_t y = (CANVAS_SIZE - (lv_coord_t)art->header.h) / 2;

    lv_draw_image_dsc_t img_dsc;
    lv_draw_image_dsc_init(&img_dsc);
    canvas_draw_img(canvas, x, y, art, &img_dsc);

    rotate_canvas(canvas);
}

void init_label_dsc(lv_draw_label_dsc_t *label_dsc, lv_color_t color, const lv_font_t *font,
                    lv_text_align_t align) {
    lv_draw_label_dsc_init(label_dsc);
    label_dsc->color = color;
    label_dsc->font = font;
    label_dsc->align = align;
}

void init_rect_dsc(lv_draw_rect_dsc_t *rect_dsc, lv_color_t bg_color) {
    lv_draw_rect_dsc_init(rect_dsc);
    rect_dsc->bg_color = bg_color;
}

void canvas_draw_rect(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                      lv_draw_rect_dsc_t *draw_dsc) {
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);
    lv_area_t coords = {x, y, x + w - 1, y + h - 1};
    lv_draw_rect(&layer, draw_dsc, &coords);
    lv_canvas_finish_layer(canvas, &layer);
}

void canvas_draw_text(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, lv_coord_t max_w,
                      lv_draw_label_dsc_t *draw_dsc, const char *txt) {
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);
    draw_dsc->text = txt;
    lv_area_t coords = {x, y, x + max_w - 1, y + CANVAS_SIZE};
    lv_draw_label(&layer, draw_dsc, &coords);
    lv_canvas_finish_layer(canvas, &layer);
}

void canvas_draw_img(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, const lv_image_dsc_t *src,
                     lv_draw_image_dsc_t *draw_dsc) {
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);
    draw_dsc->src = src;
    lv_area_t coords = {x, y, x + src->header.w - 1, y + src->header.h - 1};
    lv_draw_image(&layer, draw_dsc, &coords);
    lv_canvas_finish_layer(canvas, &layer);
}
