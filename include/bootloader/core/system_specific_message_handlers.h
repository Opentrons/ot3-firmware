#pragma once

#include "bootloader/core/message_handler.h"

HandleMessageReturn system_sepcific_handle_message(const Message* request, Message* response);
