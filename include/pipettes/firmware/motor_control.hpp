#pragma once


/**
 * Implementation of the Motor concept.
 *
 * TODO (2021-06-29): Issue SPI commands to motor.
 */
struct MotorControl {
    void set_speed(uint32_t s) {speed = s;}
    [[nodiscard]] auto get_speed() const -> uint32_t  {return speed;}

private:
    uint32_t speed{0};
};
