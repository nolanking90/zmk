/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_keymap_changes

#include <zephyr/device.h>
#include <zephyr/settings/settings.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/keymap.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

enum keymap_changes_op {
    KEYMAP_CHANGES_DISCARD = 0,
    KEYMAP_CHANGES_SAVE = 1,
    KEYMAP_CHANGES_RESET = 2,
    KEYMAP_CHANGES_STORE_STALE_SETTINGS = 3,
};

static int on_keymap_changes_binding_pressed(struct zmk_behavior_binding *binding,
                                             struct zmk_behavior_binding_event event) {
    int result;

    switch (binding->param1) {
    case KEYMAP_CHANGES_DISCARD:
        result = zmk_keymap_discard_changes();
        break;
    case KEYMAP_CHANGES_SAVE:
        result = zmk_keymap_save_changes();
        break;
    case KEYMAP_CHANGES_RESET:
        result = zmk_keymap_reset_settings();
        break;
    case KEYMAP_CHANGES_STORE_STALE_SETTINGS: {
        // As if saved by firmware with more layers than this one.
        const zmk_behavior_local_id_t id = ZMK_BEHAVIOR_LOCAL_ID_NONE;
        result = settings_save_one("keymap/l/250/0", &id, sizeof(id));
        if (result >= 0) {
            result = settings_save_one("keymap/s/250/0", &id, sizeof(id));
        }
        break;
    }
    default:
        result = -EINVAL;
        break;
    }

    if (result < 0) {
        LOG_ERR("Keymap changes operation %d failed (err: %d)", binding->param1, result);
        return result;
    }

    LOG_DBG("Keymap changes operation %d done", binding->param1);

    return 0;
}

static int on_keymap_changes_binding_released(struct zmk_behavior_binding *binding,
                                              struct zmk_behavior_binding_event event) {
    return 0;
}

static const struct behavior_driver_api behavior_keymap_changes_driver_api = {
    .binding_pressed = on_keymap_changes_binding_pressed,
    .binding_released = on_keymap_changes_binding_released};

#define KEYMAP_CHANGES_INST(n)                                                                     \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                   \
                            &behavior_keymap_changes_driver_api);

DT_INST_FOREACH_STATUS_OKAY(KEYMAP_CHANGES_INST)

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
