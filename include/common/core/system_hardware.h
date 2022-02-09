#pragma once

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * Enter the bootloader. Defined here but implemented in common firmware and
 * simulation.
 */
void enter_bootloader();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
