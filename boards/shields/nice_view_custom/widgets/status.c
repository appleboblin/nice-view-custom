/*
 * Central / non-split status screen.
 *
 *   top    (68 px): output status, own battery
 *   middle (68 px): art image
 *   bottom (24 px): BLE profile dots, active layer name
 *
 * Copyright (c) 2023-2025 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/display.h>
#include <zmk/event_manager.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>

#include "status.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct output_status_state {
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
    bool profiles_connected[NVC_PROFILE_COUNT];
    bool profiles_bonded[NVC_PROFILE_COUNT];
};

struct layer_status_state {
    zmk_keymap_layer_index_t index;
    const char *label;
};

/* ---------- drawing ---------- */

static void draw_top(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 0);

    lv_draw_label_dsc_t icon_dsc;
    init_label_dsc(&icon_dsc, LVGL_FOREGROUND, &lv_font_montserrat_16, LV_TEXT_ALIGN_RIGHT);
    lv_draw_label_dsc_t text_dsc;
    init_label_dsc(&text_dsc, LVGL_FOREGROUND, &lv_font_unscii_8, LV_TEXT_ALIGN_LEFT);
    lv_draw_rect_dsc_t fg;
    init_rect_dsc(&fg, LVGL_FOREGROUND);

    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    /* Output status: transport text on the left, state icon on the right */
    char transport[16] = "";
    const char *icon = "";
    switch (state->selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB:
        strcpy(transport, "USB");
        icon = LV_SYMBOL_USB;
        break;
    case ZMK_TRANSPORT_BLE:
        snprintf(transport, sizeof(transport), "BT %d", state->active_profile_index + 1);
        if (state->active_profile_bonded) {
            icon = state->active_profile_connected ? LV_SYMBOL_WIFI : LV_SYMBOL_CLOSE;
        } else {
            icon = LV_SYMBOL_SETTINGS; /* open profile, advertising */
        }
        break;
    default:
        strcpy(transport, "---");
        break;
    }
    canvas_draw_text(canvas, 0, 5, CANVAS_SIZE, &text_dsc, transport);
    canvas_draw_text(canvas, 0, 1, CANVAS_SIZE, &icon_dsc, icon);

    /* divider */
    canvas_draw_rect(canvas, 0, 20, CANVAS_SIZE, 1, &fg);

    /* Battery */
    draw_battery_row(canvas, 36, state->battery, state->charging);

    rotate_canvas(canvas);
}

static void draw_bottom(lv_obj_t *widget, const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 2);

    lv_draw_label_dsc_t label_dsc;
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_12, LV_TEXT_ALIGN_CENTER);
    lv_draw_rect_dsc_t filled;
    init_rect_dsc(&filled, LVGL_FOREGROUND);
    filled.radius = LV_RADIUS_CIRCLE;
    lv_draw_rect_dsc_t ring;
    init_rect_dsc(&ring, LVGL_BACKGROUND);
    ring.radius = LV_RADIUS_CIRCLE;
    ring.border_color = LVGL_FOREGROUND;
    ring.border_width = 1;
    ring.border_opa = LV_OPA_COVER;

    lv_canvas_fill_bg(canvas, LVGL_BACKGROUND, LV_OPA_COVER);

    /* Profile dots: filled = active, ring = bonded, small dot = open */
    const int d = 7, gap = 5;
    const int x0 = (CANVAS_SIZE - (NVC_PROFILE_COUNT * d + (NVC_PROFILE_COUNT - 1) * gap)) / 2;
    for (int i = 0; i < NVC_PROFILE_COUNT; i++) {
        int x = x0 + i * (d + gap);
        if (i == state->active_profile_index) {
            canvas_draw_rect(canvas, x, 1, d, d, &filled);
        } else if (state->profiles_bonded[i]) {
            canvas_draw_rect(canvas, x, 1, d, d, &ring);
        } else {
            canvas_draw_rect(canvas, x + 2, 3, 3, 3, &filled);
        }
    }

    /* Active layer */
    if (state->layer_label == NULL || strlen(state->layer_label) == 0) {
        char text[12];
        snprintf(text, sizeof(text), "Layer %u", state->layer_index);
        canvas_draw_text(canvas, 0, 9, CANVAS_SIZE, &label_dsc, text);
    } else {
        canvas_draw_text(canvas, 0, 9, CANVAS_SIZE, &label_dsc, state->layer_label);
    }

    rotate_canvas(canvas);
}

/* ---------- own battery ---------- */

static void set_battery_status(struct zmk_widget_status *widget,
                               struct battery_status_state state) {
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    widget->state.charging = state.usb_present;
#endif
    widget->state.battery = state.level;
    draw_top(widget->obj, &widget->state);
}

static void battery_status_update_cb(struct battery_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_status(widget, state); }
}

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);
    return (struct battery_status_state){
        .level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif
    };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif

/* ---------- output / profiles ---------- */

static void set_output_status(struct zmk_widget_status *widget,
                              const struct output_status_state *state) {
    widget->state.selected_endpoint = state->selected_endpoint;
    widget->state.active_profile_index = state->active_profile_index;
    widget->state.active_profile_connected = state->active_profile_connected;
    widget->state.active_profile_bonded = state->active_profile_bonded;
    for (int i = 0; i < NVC_PROFILE_COUNT; ++i) {
        widget->state.profiles_connected[i] = state->profiles_connected[i];
        widget->state.profiles_bonded[i] = state->profiles_bonded[i];
    }
    draw_top(widget->obj, &widget->state);
    draw_bottom(widget->obj, &widget->state);
}

static void output_status_update_cb(struct output_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_output_status(widget, &state); }
}

static struct output_status_state output_status_get_state(const zmk_event_t *_eh) {
    struct output_status_state state = {
        .selected_endpoint = zmk_endpoint_get_selected(),
#if defined(CONFIG_ZMK_BLE)
        .active_profile_index = zmk_ble_active_profile_index(),
        .active_profile_connected = zmk_ble_active_profile_is_connected(),
        .active_profile_bonded = !zmk_ble_active_profile_is_open(),
#else
        .active_profile_index = -1,
#endif
    };
#if defined(CONFIG_ZMK_BLE)
    for (int i = 0; i < MIN(NVC_PROFILE_COUNT, ZMK_BLE_PROFILE_COUNT); ++i) {
        state.profiles_connected[i] = zmk_ble_profile_is_connected(i);
        state.profiles_bonded[i] = !zmk_ble_profile_is_open(i);
    }
#endif
    return state;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, output_status_get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

/* ---------- layer ---------- */

static void set_layer_status(struct zmk_widget_status *widget, struct layer_status_state state) {
    widget->state.layer_index = state.index;
    widget->state.layer_label = state.label;
    draw_bottom(widget->obj, &widget->state);
}

static void layer_status_update_cb(struct layer_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_status(widget, state); }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){
        .index = index, .label = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index))};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)
ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

/* ---------- init ---------- */

int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent) {
    widget->obj = lv_obj_create(parent);
    init_canvases(widget->obj, widget->cbuf, widget->cbuf2, widget->cbuf3);

    draw_art(lv_obj_get_child(widget->obj, 1));

    sys_slist_append(&widgets, &widget->node);
    widget_battery_status_init();
    widget_output_status_init();
    widget_layer_status_init();

    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) { return widget->obj; }
