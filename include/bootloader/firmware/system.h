#pragma once

/**
 * Initialize hardware and clocks. Defined in system_stmXXXX.c.
 */
void HardwareInit(void);

// May be defined on some systems. Has a weakly-defined default.
void system_specific_startup(void);
