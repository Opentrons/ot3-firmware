#pragma once

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void limit_switch_gpio_init();
void LED_drive_gpio_init();
void utility_gpio_init();
void sync_drive_gpio_init();
int tip_present();
void tip_sense_gpio_init();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus