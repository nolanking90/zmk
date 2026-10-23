/*
 * Copyright (c) 2022 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_sensor_rotate_var

#include <zephyr/device.h>

#include <drivers/behavior.h>

#include <zephyr/logging/log.h>

#include "behavior_sensor_rotate_common.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

// param1 is passed to the clockwise binding and param2 to the counter-clockwise one, so each
// takes its values from the first parameter of that binding's behavior (e.g. a key for &kp).
static int
sensor_rotate_var_parameter_metadata(const struct device *dev,
                                     struct behavior_parameter_metadata *param_metadata) {
    const struct behavior_sensor_rotate_config *cfg = dev->config;
    struct behavior_sensor_rotate_data *data = dev->data;
    struct behavior_parameter_metadata child_meta;
    int err;

    err = behavior_get_parameter_metadata(zmk_behavior_get_binding(cfg->cw_binding.behavior_dev),
                                          &child_meta);
    if (err < 0) {
        LOG_WRN("Failed to get the clockwise behavior parameter: %d", err);
        return err;
    }

    if (child_meta.sets_len > 0) {
        data->set.param1_values = child_meta.sets[0].param1_values;
        data->set.param1_values_len = child_meta.sets[0].param1_values_len;
    }

    err = behavior_get_parameter_metadata(zmk_behavior_get_binding(cfg->ccw_binding.behavior_dev),
                                          &child_meta);
    if (err < 0) {
        LOG_WRN("Failed to get the counter-clockwise behavior parameter: %d", err);
        return err;
    }

    if (child_meta.sets_len > 0) {
        data->set.param2_values = child_meta.sets[0].param1_values;
        data->set.param2_values_len = child_meta.sets[0].param1_values_len;
    }

    param_metadata->sets = &data->set;
    param_metadata->sets_len = 1;

    return 0;
}

#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)

static const struct behavior_driver_api behavior_sensor_rotate_var_driver_api = {
    .sensor_binding_accept_data = zmk_behavior_sensor_rotate_common_accept_data,
    .sensor_binding_process = zmk_behavior_sensor_rotate_common_process,
#if IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
    .get_parameter_metadata = sensor_rotate_var_parameter_metadata,
#endif // IS_ENABLED(CONFIG_ZMK_BEHAVIOR_METADATA)
};

#define SENSOR_ROTATE_VAR_INST(n)                                                                  \
    static struct behavior_sensor_rotate_config behavior_sensor_rotate_var_config_##n = {          \
        .cw_binding = {.behavior_dev = DEVICE_DT_NAME(DT_INST_PHANDLE_BY_IDX(n, bindings, 0))},    \
        .ccw_binding = {.behavior_dev = DEVICE_DT_NAME(DT_INST_PHANDLE_BY_IDX(n, bindings, 1))},   \
        .tap_ms = DT_INST_PROP(n, tap_ms),                                                         \
        .override_params = true,                                                                   \
    };                                                                                             \
    static struct behavior_sensor_rotate_data behavior_sensor_rotate_var_data_##n = {};            \
    BEHAVIOR_DT_INST_DEFINE(n, NULL, NULL, &behavior_sensor_rotate_var_data_##n,                   \
                            &behavior_sensor_rotate_var_config_##n, POST_KERNEL,                   \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                   \
                            &behavior_sensor_rotate_var_driver_api);

DT_INST_FOREACH_STATUS_OKAY(SENSOR_ROTATE_VAR_INST)
