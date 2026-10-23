/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_discard_keymap_changes

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/keymap.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

static int on_discard_keymap_changes_binding_pressed(struct zmk_behavior_binding *binding,
                                                     struct zmk_behavior_binding_event event) {
    int result = zmk_keymap_discard_changes();

    if (result < 0) {
        LOG_ERR("Failed to discard keymap changes (err: %d)", result);
        return result;
    }

    LOG_DBG("Discarded keymap changes");

    return 0;
}

static int on_discard_keymap_changes_binding_released(struct zmk_behavior_binding *binding,
                                                      struct zmk_behavior_binding_event event) {
    return 0;
}

static const struct behavior_driver_api behavior_discard_keymap_changes_driver_api = {
    .binding_pressed = on_discard_keymap_changes_binding_pressed,
    .binding_released = on_discard_keymap_changes_binding_released};

#define DISCARD_KEYMAP_CHANGES_INST(n)                                                             \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, NULL, NULL, POST_KERNEL,                                \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                   \
                            &behavior_discard_keymap_changes_driver_api);

DT_INST_FOREACH_STATUS_OKAY(DISCARD_KEYMAP_CHANGES_INST)

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
