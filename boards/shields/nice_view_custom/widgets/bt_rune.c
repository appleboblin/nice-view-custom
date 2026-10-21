/* Bluetooth rune, drawn on the top-band canvas (portrait, not rotated). -- 10x16, 1 bit */

#include <lvgl.h>

#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

#ifndef LV_ATTRIBUTE_IMG_BT_RUNE
#define LV_ATTRIBUTE_IMG_BT_RUNE
#endif

const LV_ATTRIBUTE_MEM_ALIGN LV_ATTRIBUTE_LARGE_CONST LV_ATTRIBUTE_IMG_BT_RUNE uint8_t bt_rune_map[] = {
#if CONFIG_NICE_VIEW_CUSTOM_INVERTED
    0x00, 0x00, 0x00, 0xff, /* index 0: background (black) */
    0xff, 0xff, 0xff, 0xff, /* index 1: foreground (white) */
#else
    0xff, 0xff, 0xff, 0xff, /* index 0: background (white) */
    0x00, 0x00, 0x00, 0xff, /* index 1: foreground (black) */
#endif

    0x0c, 0x00, 0x0e, 0x00, 0x0f, 0x00, 0x0d, 0x80, 0xcc, 0xc0, 0x6d, 0x80, 0x3f, 0x00, 0x1e, 0x00,
    0x1e, 0x00, 0x3f, 0x00, 0x6d, 0x80, 0xcc, 0xc0, 0x0d, 0x80, 0x0f, 0x00, 0x0e, 0x00, 0x0c, 0x00,
};

const lv_image_dsc_t bt_rune = {
    .header.magic = LV_IMAGE_HEADER_MAGIC,
    .header.cf = LV_COLOR_FORMAT_I1,
    .header.w = 10,
    .header.h = 16,
    .header.stride = 2,
    .data_size = sizeof(bt_rune_map),
    .data = bt_rune_map,
};
