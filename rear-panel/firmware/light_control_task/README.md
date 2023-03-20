# Light Control Task

The light control task accepts messages that control the behavior of the LED's. A FreeRTOS timer is configured to run at a regular interval and send a message that instructs the light control task to update the LED's based on the current animation settings. This implicitly drives the update frequency of 200Hz, or 5ms per update.