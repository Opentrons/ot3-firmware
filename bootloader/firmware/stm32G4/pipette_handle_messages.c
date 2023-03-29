#include <string.h> // memcpy

#include "bootloader/core/message_handler.h"
#include "bootloader/core/ids.h"
#include "bootloader/core/pipette_type.h"
#include "bootloader/core/node_id.h"
#include "common/core/version.h"
#include "bootloader/core/messages.h"
#include "bootloader/core/util.h"

static HandleMessageReturn handle_device_info_request(
    const Message* request, Message* response);

HandleMessageReturn system_specific_handle_message(
    const Message* request, Message* response) {
    switch (request->arbitration_id.parts.message_id) {
        case can_messageid_device_info_request:
            return handle_device_info_request(request, response);
        default:
            return handle_message_not_handled;
    }
}

static CANPipetteType can_pipette_type_for_internal(PipetteType type);

HandleMessageReturn handle_device_info_request(
    const Message* request, Message* response) {
    HandleMessageReturn system_ret = system_handle_message(request, response);
    if (system_ret != handle_message_has_response) {
        return system_ret;
    }
    size_t response_index =
        sizeof(*version_get())   // all version info is in that struct
        + sizeof(uint32_t) // message index
        + revision_size(); // revision
    uint8_t* pipette_id_loc = response->data + response_index;
    *pipette_id_loc = can_pipette_type_for_internal(PIPETTE_TYPE_DEFINE);
    return handle_message_has_response;
}

CANPipetteType can_pipette_type_for_internal(PipetteType type) {
    switch (type) {
        case SINGLE_CHANNEL:
            return can_pipettetype_pipette_single;
        case EIGHT_CHANNEL:
            return can_pipettetype_pipette_multi;
        case NINETY_SIX_CHANNEL:
            return can_pipettetype_pipette_96;
        default:
            return 0;
    }
}
