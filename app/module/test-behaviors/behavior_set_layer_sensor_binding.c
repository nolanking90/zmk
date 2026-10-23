/*
 * Copyright (c) 2026 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_set_layer_sensor_binding

#include <zephyr/device.h>
#include <drivers/behavior.h>
#include <zephyr/logging/log.h>

#include <zmk/keymap.h>
#include <zmk/behavior.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#if DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)

struct behavior_set_layer_sensor_binding_config {
    size_t bindings_len;
    const struct zmk_behavior_binding *bindings;
    bool clear;
};

struct behavior_set_layer_sensor_binding_data {
    size_t current_idx;
};

static int on_set_layer_sensor_binding_pressed(struct zmk_behavior_binding *binding,
                                               struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    const struct behavior_set_layer_sensor_binding_config *cfg = dev->config;
    struct behavior_set_layer_sensor_binding_data *data = dev->data;

    if (cfg->bindings_len == 0) {
        LOG_ERR("No sensor bindings configured");
        return -EINVAL;
    }

    const struct zmk_behavior_binding empty = {0};
    int result = zmk_keymap_set_layer_sensor_binding(
        binding->param1, binding->param2, cfg->clear ? empty : cfg->bindings[data->current_idx]);

    if (result < 0) {
        LOG_ERR("Failed to set sensor binding at layer %d, sensor %d (err: %d)", binding->param1,
                binding->param2, result);
        return result;
    }

    if (cfg->clear) {
        LOG_DBG("Cleared sensor binding at layer %d, sensor %d", binding->param1, binding->param2);
        return 0;
    }

    LOG_DBG("Set sensor binding at layer %d, sensor %d to binding %zu/%zu", binding->param1,
            binding->param2, data->current_idx + 1, cfg->bindings_len);

    // Move to next binding, wrap around if at end
    data->current_idx = (data->current_idx + 1) % cfg->bindings_len;

    return 0;
}

static int on_set_layer_sensor_binding_released(struct zmk_behavior_binding *binding,
                                                struct zmk_behavior_binding_event event) {
    return 0;
}

static const struct behavior_driver_api behavior_set_layer_sensor_binding_driver_api = {
    .binding_pressed = on_set_layer_sensor_binding_pressed,
    .binding_released = on_set_layer_sensor_binding_released};

static int behavior_set_layer_sensor_binding_init(const struct device *dev) {
    struct behavior_set_layer_sensor_binding_data *data = dev->data;
    data->current_idx = 0;
    return 0;
};

#define _TRANSFORM_SENSOR_ENTRY(idx, node)                                                         \
    {                                                                                              \
        .behavior_dev = DEVICE_DT_NAME(DT_PHANDLE_BY_IDX(node, sensor_bindings, idx)),             \
        .param1 = COND_CODE_0(DT_PHA_HAS_CELL_AT_IDX(node, sensor_bindings, idx, param1), (0),     \
                              (DT_PHA_BY_IDX(node, sensor_bindings, idx, param1))),                \
        .param2 = COND_CODE_0(DT_PHA_HAS_CELL_AT_IDX(node, sensor_bindings, idx, param2), (0),     \
                              (DT_PHA_BY_IDX(node, sensor_bindings, idx, param2))),                \
    }

#define TRANSFORMED_BINDINGS(n)                                                                    \
    {LISTIFY(DT_INST_PROP_LEN(n, sensor_bindings), _TRANSFORM_SENSOR_ENTRY, (, ), DT_DRV_INST(n))}

#define SET_LAYER_SENSOR_BINDING_INST(n)                                                           \
    static struct behavior_set_layer_sensor_binding_data                                           \
        behavior_set_layer_sensor_binding_data_##n = {};                                           \
                                                                                                   \
    static const struct zmk_behavior_binding                                                       \
        behavior_set_layer_sensor_binding_config_##n##_bindings[DT_INST_PROP_LEN(                  \
            n, sensor_bindings)] = TRANSFORMED_BINDINGS(n);                                        \
    static const struct behavior_set_layer_sensor_binding_config                                   \
        behavior_set_layer_sensor_binding_config_##n = {                                           \
            .bindings_len = DT_INST_PROP_LEN(n, sensor_bindings),                                  \
            .bindings = behavior_set_layer_sensor_binding_config_##n##_bindings,                   \
            .clear = DT_INST_PROP(n, clear)};                                                      \
                                                                                                   \
    BEHAVIOR_DT_INST_DEFINE(n, behavior_set_layer_sensor_binding_init, NULL,                       \
                            &behavior_set_layer_sensor_binding_data_##n,                           \
                            &behavior_set_layer_sensor_binding_config_##n, POST_KERNEL,            \
                            CONFIG_KERNEL_INIT_PRIORITY_DEFAULT,                                   \
                            &behavior_set_layer_sensor_binding_driver_api);

DT_INST_FOREACH_STATUS_OKAY(SET_LAYER_SENSOR_BINDING_INST)

#endif // DT_HAS_COMPAT_STATUS_OKAY(DT_DRV_COMPAT)
