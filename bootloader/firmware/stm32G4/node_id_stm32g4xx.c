#include <stdbool.h>
#include <stdint.h>
#include "bootloader/core/node_id.h"
#include "stm32g4xx_hal.h"

#if defined(node_id_pipette_dynamic)


static CANNodeId dynamic_id_backing = can_nodeid_broadcast;


static CANNodeId update_dynamic_nodeid() {
    // B0: mount id
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_Delay(2);
    int level = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0);
    CANNodeId id = determine_pipette_node_id(level == GPIO_PIN_RESET);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(
        GPIOB,
        GPIO_PIN_0,
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
    return can_nodeid_pipette_right_bootloader
#elif defined(node_id_pipette_dynamic)
    return get_dynamic_nodeid();
#else
#error "No node id"
#endif
}
