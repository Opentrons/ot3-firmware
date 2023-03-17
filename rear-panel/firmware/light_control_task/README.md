# Light Control Task

The light control task runs every millisecond to update the LED's. The task also accepts messages that control the behavior of the LED's.

- When the task runs, it checks if there is a new message.
  - _If_ the incoming queue is not empty, the first message is consumed
- The LED statuses are updated based on the current LED action.
