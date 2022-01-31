#include "bootloader/core/message_handler.h"
#include "bootloader/core/ids.h"
#include "bootloader/core/node_id.h"
#include "bootloader/core/version.h"
#include "bootloader/core/messages.h"


static HandleMessageReturn handle_device_info_request(const Message* request, Message* response);

static HandleMessageReturn handle_initiate_fw_update(const Message* request, Message* response);

static HandleMessageReturn handle_fw_update_data(const Message* request, Message* response);

static HandleMessageReturn handle_fw_update_complete(const Message* request, Message* response);

static uint8_t * write_uint16(uint8_t * buffer, uint16_t value);

static uint8_t * write_uint32(uint8_t * buffer, uint32_t value);

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

    HandleMessageReturn ret = handle_message_ok;

    switch (request->arbitration_id.parts.message_id) {
        case can_messageid_fw_update_data:
            ret = handle_fw_update_data(request, response);
            break;
        case can_messageid_fw_update_complete:
            ret = handle_fw_update_complete(request, response);
            break;
        case can_messageid_fw_update_initiate:
            ret = handle_initiate_fw_update(request, response);
            break;
        case can_messageid_device_info_request:
            ret = handle_device_info_request(request, response);
            break;
        default:
            break;
    }
    return ret;
}


HandleMessageReturn handle_device_info_request(const Message* request, Message* response) {
    response->arbitration_id.id = 0;
    response->arbitration_id.parts.message_id = can_messageid_device_info_response;
    response->arbitration_id.parts.node_id = can_nodeid_host;
    response->arbitration_id.parts.originating_node_id = get_node_id();
    response->size = 4;
    write_uint32(response->data, get_version());
    return handle_message_has_response;
}


HandleMessageReturn handle_initiate_fw_update(const Message* request, Message* response) {

    return handle_message_ok;
}

HandleMessageReturn handle_fw_update_data(const Message* request, Message* response) {
    UpdateData data;
    ErrorCode e = parse_update_data(request->data, request->size, &data);

    if (e == can_errorcode_ok) {
        // All is good. Pass on to flasher.
    }

    // Build response
    uint8_t* p = response->data;
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
    ErrorCode e = parse_update_complete(request->data, request->size, &complete);

    if (e == can_errorcode_ok) {
        e = can_errorcode_invalid_size;
    }

    // Build response
    uint8_t* p = response->data;
    p = write_uint16(p, e);

    response->arbitration_id.id = 0;
    response->arbitration_id.parts.message_id = can_messageid_fw_update_complete_ack;
    response->arbitration_id.parts.node_id = can_nodeid_host;
    response->arbitration_id.parts.originating_node_id = get_node_id();
    response->size = p - response->data;

    return handle_message_has_response;
}


uint8_t * write_uint16(uint8_t * buffer, uint16_t value) {
    *(buffer++) = (value >> 8) & 0xFF;
    *(buffer++) = value & 0xFF;
    return buffer;
}

uint8_t * write_uint32(uint8_t * buffer, uint32_t value) {
    *(buffer++) = (value >> 24) & 0xFF;
    *(buffer++) = (value >> 16) & 0xFF;
    *(buffer++) = (value >> 8) & 0xFF;
    *(buffer++) = value & 0xFF;
    return buffer;
}
