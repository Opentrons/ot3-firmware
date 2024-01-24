#include <stdbool.h>
#include <stdint.h>
#include "bootloader/core/node_id.h"
#include "bootloader/core/pipette_type.h"
#include "stm32g4xx_hal.h"

#if defined(node_id_pipette_dynamic)

static CANNodeId dynamic_id_backing = can_nodeid_broadcast;

static CANNodeId determine_pipette_node_id(
    bool mount_id) {
    return mount_id ? can_nodeid_pipette_left_bootloader : can_nodeid_pipette_right_bootloader;
}


static CANNodeId update_dynamic_nodeid() {
    PipetteType pipette_type = get_pipette_type();
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    void* port;
    if (pipette_type == NINETY_SIX_CHANNEL) {
        // C3: mount id
        __HAL_RCC_GPIOC_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_3;
        port = GPIOC;
    } else {
        // B0: mount id
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        port = GPIOB;
    }

    HAL_GPIO_Init(port, &GPIO_InitStruct);
    HAL_Delay(100);
    int level = HAL_GPIO_ReadPin(port, GPIO_InitStruct.Pin);
    CANNodeId id = determine_pipette_node_id(level == GPIO_PIN_RESET);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(
        port,
        GPIO_InitStruct.Pin,
        ((level == GPIO_PIN_SET) ? GPIO_PIN_RESET : GPIO_PIN_SET));
    return id;
}

static CANNodeId get_dynamic_nodeid() {
    if (dynamic_id_backing == can_nodeid_broadcast) {
        dynamic_id_backing = update_dynamic_nodeid();
    }
    return dynamic_id_backing;
}


#endif // #if defined(node_id_pipette_dynamic)

/**
 * Get the node id this bootloader is installed on
 * @return Node id.
 */
CANNodeId get_node_id(void) {
#if defined node_id_head
    return can_nodeid_head_bootloader;
#elif defined node_id_gantry_x
    return can_nodeid_gantry_x_bootloader;
#elif defined node_id_gantry_y
    return can_nodeid_gantry_y_bootloader;
#elif defined node_id_gripper
    return can_nodeid_gripper_bootloader;
#elif defined(node_id_pipette_left)
    return can_nodeid_pipette_left_bootloader;
#elif defined(node_id_pipette_right)
    return can_nodeid_pipette_right_bootloader;
#elif defined(node_id_pipette_dynamic)
    return get_dynamic_nodeid();
#elif defined(node_id_hepa_uv)
    return can_nodeid_hepa_uv_bootloader;
#else
#error "No node id"
#endif
}
