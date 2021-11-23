



spi::SPI_interface SPI_intf = {
    .SPI_handle = &hspi2,
    .GPIO_handle = GPIOB,
    .pin = GPIO_PIN_12,
    };

static spi::Spi spi_comms(SPI_intf);


struct motion_controller::HardwareConfig PinConfigurations = {
    .direction = {.port = GPIOB,
                  .pin = GPIO_PIN_1,
                  .active_setting = GPIO_PIN_SET},
                  .step = {.port = GPIOC, .pin = GPIO_PIN_8, .active_setting = GPIO_PIN_SET},
                  .enable = {
        .port = GPIOA, .pin = GPIO_PIN_9, .active_setting = GPIO_PIN_SET},
        };