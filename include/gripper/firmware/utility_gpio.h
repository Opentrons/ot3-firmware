#pragma once

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void utility_gpio_init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus


#if PCBA_PRIMARY_REVISION == 'b' || PCBA_PRIMARY_REVISION == 'a'
#define LED_GPIO_PIN GPIO_PIN_6
#define Z_LIM_SW_PIN GPIO_PIN_7
#define NSYNC_OUT_PORT GPIOB
#define NSYNC_OUT_PIN GPIO_PIN_6
#else
#define LED_GPIO_PIN GPIO_PIN_10
#define Z_LIM_SW_PIN GPIO_PIN_13
#define NSYNC_OUT_PORT GPIOD
#define NSYNC_OUT_PIN GPIO_PIN_2
#define EBRAKE_PIN GPIO_PIN_5
#define EBRAKE_PORT GPIOB
#endif

//common defines for above pins
#define LED_GPIO_PORT GPIOC
#define Z_LIM_SW_PORT GPIOC
//general gpio pins
#define NSYNC_IN_PIN GPIO_PIN_7
#define NSYNC_IN_PORT GPIOB
#define ESTOP_IN_PIN GPIO_PIN_10
#define ESTOP_IN_PORT GPIOA
#define TOOL_DETECT_PIN GPIO_PIN_0
#define TOOL_DETECT_PORT GPIOB
// g motor pins
#define G_MOT_ENABLE_PIN GPIO_PIN_11
#define G_MOT_ENABLE_PORT GPIOC
#define G_MOT_PWM_CH1_PIN GPIO_PIN_0
#define G_MOT_PWM_CH1_PORT GPIOC
#define G_MOT_PWM_CH2_PIN GPIO_PIN_6
#define G_MOT_PWM_CH2_PORT GPIOA
#define G_MOT_VREF_PIN GPIO_PIN_4
#define G_MOT_VREF_PORT GPIOA
#define G_LIM_SW_PIN GPIO_PIN_2
#define G_LIM_SW_PORT GPIOC
#define G_MOT_ENC_A_PIN GPIO_PIN_0
#define G_MOT_ENC_B_PIN GPIO_PIN_1
#define G_MOT_ENC_I_PIN GPIO_PIN_5
#define G_MOT_ENC_PORT GPIOA
// z motor pins
#define Z_MOT_DIR_PIN GPIO_PIN_10
#define Z_MOT_STEP_PIN GPIO_PIN_1
#define Z_MOT_DIAG0_PIN GPIO_PIN_2
#define Z_MOT_STEPDIR_PORT GPIOB
#define Z_MOT_ENABLE_PIN GPIO_PIN_9
#define Z_MOT_ENABLE_PORT GPIOA
#define Z_MOT_DRIVE_CS GPIO_PIN_12
#define Z_MOT_DRIVE_CLK GPIO_PIN_13
#define Z_MOT_DRIVE_MISO GPIO_PIN_14
#define Z_MOT_DRIVE_MOSI GPIO_PIN_15
#define Z_MOT_DRIVE_PORT GPIOB
#define Z_MOT_ENC_A_PIN GPIO_PIN_6
#define Z_MOT_ENC_B_PIN GPIO_PIN_7
#define Z_MOT_ENC_AB_PORT GPIOC
#define Z_MOT_ENC_I_PIN GPIO_PIN_6
#define Z_MOT_ENC_I_PORT GPIOB
