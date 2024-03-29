#include <stddef.h> // offsetof
#include <string.h> // memcpy
#include "bootloader/core/message_handler.h"
#include "bootloader/core/ids.h"
#include "bootloader/core/node_id.h"
#include "bootloader/core/messages.h"
#include "bootloader/core/util.h"
#include "bootloader/core/updater.h"
#include "bootloader/core/update_state.h"
#include "common/core/app_update.h"
#include "common/core/version.h"
#include "bootloader/core/system_specific_message_handlers.h"

/** Handle a device info request message. */
static HandleMessageReturn handle_device_info_request(const Message* request, Message* response);

/** Handle an initiate firmware update message. */
static HandleMessageReturn handle_initiate_fw_update(const Message* request, Message* response);

/** Handle a chunk of the firmware update data. */
static HandleMessageReturn handle_fw_update_data(const Message* request, Message* response);

/** Handle a firmware update complete message. */
static HandleMessageReturn handle_fw_update_complete(const Message* request, Message* response);

/** Handle a request to get firmware update status. */
static HandleMessageReturn handle_fw_update_status_request(const Message* request, Message* response);

/** Handle a request to erase an application from flash. */
static HandleMessageReturn handle_fw_update_erase_application(const Message* request, Message* response);

// Default implementation - override in a source file if you want to use it.
HandleMessageReturn __attribute__((weak)) system_specific_handle_message(const Message* request, Message* response) {
    (void)request;
    (void)response;
    return handle_message_not_handled;
}

/**
 * Handle a message
 * @param request The request
 * @param response Where to put the response.
 * @return Return code
 */
HandleMessageReturn handle_message(const Message* request, Message* response) {
    if (!request || !response) {
        return handle_message_error;
    }

    const HandleMessageReturn ret = system_specific_handle_message(request, response);
    if (ret != handle_message_not_handled) {
        return ret;
    }
    return system_handle_message(request, response);
}

HandleMessageReturn system_handle_message(const Message* request, Message* response) {
    switch (request->arbitration_id.parts.message_id) {
        case can_messageid_fw_update_data:
            return handle_fw_update_data(request, response);
        case can_messageid_fw_update_complete:
            return handle_fw_update_complete(request, response);
        case can_messageid_fw_update_initiate:
            return handle_initiate_fw_update(request, response);
        case can_messageid_device_info_request:
            return handle_device_info_request(request, response);
        case can_messageid_fw_update_status_request:
            return handle_fw_update_status_request(request, response);
        case can_messageid_fw_update_start_app:
            fw_update_start_application();
            return handle_message_ok;
        case can_messageid_fw_update_erase_app:
            return handle_fw_update_erase_application(request, response);
        default:
            // No error, we didn't have anything to do.
            return handle_message_ok;
    }
}


HandleMessageReturn handle_device_info_request(const Message* request, Message* response) {
    response->arbitration_id.id = 0;
    response->arbitration_id.parts.message_id = can_messageid_device_info_response;
    response->arbitration_id.parts.node_id = can_nodeid_host;
    response->arbitration_id.parts.originating_node_id = get_node_id();
    const struct version *vstruct = version_get();
    const struct revision *rstruct = revision_get();
    uint32_t message_index;
    parse_empty_message(request->data, request->size, &message_index);
    response->size =
        sizeof(*vstruct)   // all version info is in that struct
        + sizeof(uint32_t) // message index
        + revision_size()  // revision
        + sizeof(char);    // device subtype
    uint8_t* p = response->data;
    p = write_uint32(p, message_index);
    p = write_uint32(p, vstruct->version);
    p = write_uint32(p, vstruct->flags);
    p = memcpy(p, &vstruct->sha[0], sizeof(vstruct->sha));
    p += sizeof(vstruct->sha);
    *p++ = rstruct->primary;
    *p++ = rstruct->secondary;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    return handle_message_has_response;
}

HandleMessageReturn handle_initiate_fw_update(const Message* request, Message* response) {
    fw_update_initialize(get_update_state());
    response->arbitration_id.id = 0;
    response->arbitration_id.parts.message_id = can_messageid_acknowledgement;
    response->arbitration_id.parts.node_id = can_nodeid_host;
    response->arbitration_id.parts.originating_node_id = get_node_id();
    uint32_t message_index;
    parse_empty_message(request->data, request->size, &message_index);
    response->size = sizeof(uint32_t);
    write_uint32(response->data, message_index);
    return handle_message_has_response;
}

HandleMessageReturn handle_fw_update_data(const Message* request, Message* response) {
    UpdateData data;
    CANErrorCode e = parse_update_data(request->data, request->size, &data);

    if (e == can_errorcode_ok) {
        // All is good. Pass on to updater.
        FwUpdateReturn updater_return = fw_update_data(get_update_state(),
                                                       data.address,
                                                       data.data,
                                                       data.num_bytes);
        if (updater_return != fw_update_ok) {
            e = can_errorcode_hardware;
        }
    }

    // Build response
    uint8_t* p = response->data;
    p = write_uint32(p, data.message_index);
    p = write_uint32(p, data.address);
    p = write_uint16(p, e);

    response->arbitration_id.id = 0;
    response->arbitration_id.parts.message_id = can_messageid_fw_update_data_ack;
    response->arbitration_id.parts.node_id = can_nodeid_host;
    response->arbitration_id.parts.originating_node_id = get_node_id();
    response->size = p - response->data;

    return handle_message_has_response;
}

HandleMessageReturn handle_fw_update_complete(const Message* request, Message* response) {
    UpdateComplete complete;
    CANErrorCode e = parse_update_complete(request->data, request->size, &complete);

    if (e == can_errorcode_ok) {
        FwUpdateReturn updater_return = fw_update_complete(
            get_update_state(),
            complete.num_messages,
            complete.crc32);
        switch (updater_return) {
            case fw_update_error:
                e = can_errorcode_hardware;
                break;
            case fw_update_invalid_data:
                e = can_errorcode_bad_checksum;
                break;
            case fw_update_invalid_size:
                e = can_errorcode_invalid_size;
                break;
            default:
                break;
        }
    }

    // Build response
    uint8_t* p = response->data;
    p = write_uint32(p, complete.message_index);
    p = write_uint16(p, e);

    response->arbitration_id.id = 0;
    response->arbitration_id.parts.message_id = can_messageid_fw_update_complete_ack;
    response->arbitration_id.parts.node_id = can_nodeid_host;
    response->arbitration_id.parts.originating_node_id = get_node_id();
    response->size = p - response->data;

    return handle_message_has_response;
}


HandleMessageReturn handle_fw_update_status_request(const Message* request, Message* response) {
    uint32_t message_index;
    parse_empty_message(request->data, request->size, &message_index);

    uint8_t* p = response->data;
    p = write_uint32(p, message_index);
    p = write_uint32(p, app_update_flags());

    response->arbitration_id.id = 0;
    response->arbitration_id.parts.message_id = can_messageid_fw_update_status_response;
    response->arbitration_id.parts.node_id = can_nodeid_host;
    response->arbitration_id.parts.originating_node_id = get_node_id();
    response->size = p - response->data;
    return handle_message_has_response;
}

static HandleMessageReturn handle_fw_update_erase_application(const Message* request, Message* response) {
    FwUpdateReturn updater_return = fw_update_erase_application(get_update_state());
    uint32_t message_index;
    parse_empty_message(request->data, request->size, &message_index);

    // Build response
    uint8_t* p = response->data;
    p = write_uint32(p, message_index);
    p = write_uint16(p, updater_return == fw_update_ok ? can_errorcode_ok : can_errorcode_hardware);

    response->arbitration_id.id = 0;
    response->arbitration_id.parts.message_id = can_messageid_fw_update_erase_app_ack;
    response->arbitration_id.parts.node_id = can_nodeid_host;
    response->arbitration_id.parts.originating_node_id = get_node_id();
    response->size = p - response->data;

    return handle_message_has_response;
}
