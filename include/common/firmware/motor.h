#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

struct PinConfig {
    void* port;
    uint16_t pin;
};

void set_pin(struct PinConfig config);
void reset_pin(struct PinConfig config);
void delay(const int seconds);
void start_interrupt();
void stop_interrupt();

#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
