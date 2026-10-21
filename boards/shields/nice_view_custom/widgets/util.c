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

void init_layout(lv_obj_t *obj, uint8_t *top, uint8_t *bottom) {
    lv_obj_set_size(obj, 160, 68);
    /* Solid background so art smaller than the slot has matching margins */
    lv_obj_set_style_bg_color(obj, LVGL_BACKGROUND, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, LV_PART_MAIN);

    /* child 0: top band canvas; only landscape x 136..159 is on screen */
    lv_obj_t *c_top = lv_canvas_create(obj);
    lv_obj_align(c_top, LV_ALIGN_TOP_LEFT, 160 - BAND, 0);
    lv_canvas_set_buffer(c_top, top, CANVAS_SIZE, CANVAS_SIZE, CANVAS_COLOR_FORMAT);

    /* child 1: art, landscape x 24..135, centered */
    const lv_image_dsc_t *art = &nice_view_custom_art;
    lv_obj_t *img = lv_image_create(obj);
    lv_image_set_src(img, art);
    lv_obj_align(img, LV_ALIGN_TOP_LEFT, BAND + (ART_H - (lv_coord_t)art->header.w) / 2,
                 (ART_W - (lv_coord_t)art->header.h) / 2);

    /* child 2: bottom band canvas; only landscape x 0..23 is on screen */
    lv_obj_t *c_bottom = lv_canvas_create(obj);
    lv_obj_align(c_bottom, LV_ALIGN_TOP_LEFT, BAND - CANVAS_SIZE, 0);
    lv_canvas_set_buffer(c_bottom, bottom, CANVAS_SIZE, CANVAS_SIZE, CANVAS_COLOR_FORMAT);
}

void draw_battery_icon(lv_obj_t *canvas, lv_coord_t x, lv_coord_t y, uint8_t level,
                       bool charging) {
    lv_draw_rect_dsc_t bg, fg;
    init_rect_dsc(&bg, LVGL_BACKGROUND);
    init_rect_dsc(&fg, LVGL_FOREGROUND);

    canvas_draw_rect(canvas, x, y, 22, 12, &fg);
    canvas_draw_rect(canvas, x + 1, y + 1, 20, 10, &bg);
    canvas_draw_rect(canvas, x + 22, y + 3, 2, 6, &fg);

    if (charging) {
        lv_draw_image_dsc_t img_dsc;
        lv_draw_image_dsc_init(&img_dsc);
        canvas_draw_img(canvas, x + 6, y - 3, &bolt, &img_dsc);
    } else {
        int fill = (MIN(level, 100) * 18 + 50) / 100;
        if (fill > 0) {
            canvas_draw_rect(canvas, x + 2, y + 2, fill, 8, &fg);
        }
    }
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
