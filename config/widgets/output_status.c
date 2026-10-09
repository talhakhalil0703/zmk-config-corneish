/*
 *
 * Copyright (c) 2021 Darryl deHaan
 * SPDX-License-Identifier: MIT
 *
 */

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include "output_status.h"
#include <zmk/event_manager.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#if IS_ENABLED(CONFIG_ZMK_USB)
#include <zmk/events/usb_conn_state_changed.h>
#endif
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

struct output_status_state {
    struct zmk_endpoint_instance selected_endpoint;
    uint8_t active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
    bool usb_connected;
};

static struct output_status_state get_state(const zmk_event_t *_eh) {
    return (struct output_status_state){
        .selected_endpoint = zmk_endpoints_selected(),
        .active_profile_index = zmk_ble_active_profile_index(),
        .active_profile_connected = zmk_ble_active_profile_is_connected(),
        .active_profile_bonded = !zmk_ble_active_profile_is_open(),
#if IS_ENABLED(CONFIG_ZMK_USB)
        .usb_connected = zmk_usb_is_hid_ready(),
#endif
    };
}

static void set_status_symbol(lv_obj_t *icon, struct output_status_state state) {
    const char *ble_status = state.active_profile_connected
                                 ? (state.selected_endpoint.transport == ZMK_TRANSPORT_BLE
                                        ? LV_SYMBOL_OK
                                        : "")
                                 : (state.active_profile_bonded ? LV_SYMBOL_CLOSE : "?");
    unsigned int profile_number = state.active_profile_index + 1;

    if (state.usb_connected) {
        const char *usb_status =
            state.selected_endpoint.transport == ZMK_TRANSPORT_USB ? LV_SYMBOL_OK : "";
        lv_label_set_text_fmt(icon, LV_SYMBOL_USB "%s " LV_SYMBOL_BLUETOOTH "%u%s",
                              usb_status, profile_number, ble_status);
    } else {
        lv_label_set_text_fmt(icon, LV_SYMBOL_BLUETOOTH "%u%s", profile_number, ble_status);
    }
}

static void output_status_update_cb(struct output_status_state state) {
    struct zmk_widget_output_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_status_symbol(widget->obj, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);
#if IS_ENABLED(CONFIG_ZMK_USB)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif
// We don't get an endpoint changed event when the active profile connects/disconnects
// but there wasn't another endpoint to switch from/to, so update on BLE events too.
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

int zmk_widget_output_status_init(struct zmk_widget_output_status *widget, lv_obj_t *parent) {
    widget->obj = lv_label_create(parent);
    lv_obj_set_width(widget->obj, 76);
    lv_obj_set_style_text_font(widget->obj, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_align(widget->obj, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_letter_space(widget->obj, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(widget->obj, 0, LV_PART_MAIN);
    lv_label_set_long_mode(widget->obj, LV_LABEL_LONG_CLIP);

    sys_slist_append(&widgets, &widget->node);

    widget_output_status_init();
    return 0;
}

lv_obj_t *zmk_widget_output_status_obj(struct zmk_widget_output_status *widget) {
    return widget->obj;
}
