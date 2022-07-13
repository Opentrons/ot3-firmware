#pragma once

#include "types.hpp"

namespace eeprom {
namespace addresses {
/*  
 * header format for the eeprom chip
 * [serial number] [revison]  [usage]  [data_revison] [lookup_table_tail]
 *   [20 bytes]    [4 bytes] [4 bytes]    [2 bytes]      [2 bytes]
 *      [                   32 bytes                       ]
 *
 *
 *  overall eeprom organization
 *  [header] [lookup table] [unallocated] [data]
 *
 * TODO (rtc 07-13-22): update this info page after implementation to use the
 * real variable names and correct for design changes
 *
 *  the eeprom driver provieds a way to add remove key-values to the lookup
 * table the lookup table grows from lower address to greater, while the data
 * gets allocated from the back forward this allows future revisions to simply
 * add new data allocations without having to reformat the chip everytime.
 *
 * this also allows us to generalize around differnt eeprom chips with different
 * memory sizes in regards to data allocation. The lookup table will be hardware
 * dependent, with each element of the table being a pair of numbers equal to
 * the data address size (i.e. two 8-bit values for an 8-bit address chip and
 * two 16-bit values on 16-bit address chips) the first stores the memory
 * address of the begining of that value and the second stores the length in
 * bytes
 *
 * the implementing client will need define a list of key-values they want
 * read/write from starting at 0 in some sort of configuration file
 *
 * to initalize a key-value table on the eeprom an initialize request will be
 * sent with [key, value_length] or optionally [key, value_length, initial
 * value]
 *
 * when they send a write-to-data request they will need to provide [key,
 * buffer, buffer_len] to completly overwrite a particular key-value this should
 * probably also be overloaded with a argument list [key, entry_offset, buffer,
 * buffer_len] to enable clients to write a subset of information for longer
 * data formats such as tables.
 *
 *
 * when a client sends a read-from-data request it will provide [key,
 * read_length, callback(response_struct)] likewise to the write it should also
 * overload with [key, entry_offset, read_length, callback(response_struct)]
 *
 * if the lookup table entry at loopup_table[2*sizeof(address)*key] has not been
 * set it will call the callback with an UNINITIALIZED error *
 *
 * when a read operation is performed, first the driver will read the value
 * memeory address at loopup_table[2*sizeof(address)*key] and the data length
 * from loopup_table[(2*sizeof(address)*key) + sizeof(address)]
 * *
 * if the memory address has not been set (filled with 0x00) it will call the
 * callback with an UNINITIALIZED error the read_length would read outside of
 * (value memory address + data length) it will call the callback with an
 * INVALID READ error otherwise the callback is called with the read data in the
 * response_struct
 *
 */

constexpr types::data_length serial_number_length = 20;
constexpr types::address serial_number_address_begin = 0;
constexpr types::address serial_number_address_end =
    serial_number_address_begin + serial_number_length;


}  // namespace addresses
}  // namespace eeprom
