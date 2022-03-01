#pragma once


#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#define IWDG_INTERVAL_MS 1000

/**
 * Initialize the independent watch odg.
 */
void MX_IWDG_Init(void);

/**
 * Refresh the independent watch dog
 * @param handle
 */
void iwdg_refresh(void);


#ifdef __cplusplus
}
#endif // __cplusplus