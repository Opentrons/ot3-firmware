#pragma once

#include "types.hpp"

namespace eeprom {
namespace addresses {

/*
 * **** header format for the eeprom chip ****
 * [serial number] [revison]  [reserved]  [data_revison] [lookup_table_tail]
 *
 * [20 bytes]    [4 bytes] [4 bytes]    [2 bytes]      [2 bytes]
 *
 * [                   32 bytes                       ]
 *
 * overall eeprom organization
 * [header] [lookup table] [unallocated] [data]
 *
 *  ****    Description ****
 * Systems using the eeprom storage should use one of several of the
 * provided accessors
 * there are 3 default accessors for every chip/board
 *
 * serial number
 * data_revision
 * revision

 * each of these has 2 main functions, write and start read. Write populates
 * that type of value into the eeprom, while start_read queries the chip for
 * data and then calls a provided listener.
 *
 * ------------General purpose filesystem-----------
 * The eeprom driver provides a way to add/remove key-values to the lookup
 * table. the lookup table grows from lower address to greater, while the data
 * gets allocated from the back forward. This allows future revisions to simply
 * add new data allocations without having to reformat the chip every time. this
 * also allows us to generalize around different eeprom chips with different
 * memory sizes in regards to data allocation.
 *
 * The lookup table will be hardware dependent, with each element of the table
 * being a pair of numbers equal to the data address size (i.e. two 8-bit values
 * for an 8-bit address chip and two 16-bit values on 16-bit address chips). The
 * first stores the memory address of the beginning of that value (as an offset
 * from the data section i.e address - 32) and the second stores the length in
 * bytes.
 *
 * The implementing client will need to define a list of key-values they want to
 *  read/write from starting at 0 in some sort of configuration file to
 * initialize a key-value table on the eeprom.
 *
 * An initialize request will be sent with [key, value_length] or optionally
 * [key, value_length, initial value]. When attemping to
 *
 * When clients send a ‘write_data’ request they will need to provide at a
 * minimum [key, buffer] to completely overwrite a particular key-value. This is
 * also overloaded to [key, length, buffer] to only write to the first ‘length’
 * bytes of the value.  The final overload is [key, length, offset, buffer] this
 * function writes ‘length’ bytes offset by ‘offset’ bytes from the beginning of
 * the value
 *
 * When a client sends a get_data request it will provide [key]. As with the
 * write it should also overload with [key, length] to read the first ‘length’
 * bytes and [key, length, offset] to read ‘length’ bytes offset by ‘offset’
 * bytes from the beginning of the value
 *
 * When a read or write operation is performed, first the driver will read the
 * value memory address at  lookup_table[2*sizeof(address)*key] and the data
 * length from lookup_table[(2*sizeof(address)*key) + sizeof(address)]. If the
 * operation attempts to read a uninitialized key ie when
 * ((2*sizeof(address)*key) > data_table_tail) it will call the callback with an
 * UNINITIALIZED error.
 *
*/

constexpr types::data_length serial_number_length = 20;
constexpr types::address serial_number_address_begin = 0;
constexpr types::address serial_number_address_end =
    serial_number_address_begin + serial_number_length;

constexpr types::data_length revision_length = 4;
constexpr types::address revision_address_begin = serial_number_address_end;
constexpr types::address revision_address_end =
    revision_address_begin + revision_length;

constexpr types::data_length reserved_length = 4;
constexpr types::address reserved_address_begin = revision_address_end;
constexpr types::address reserved_address_end =
    reserved_address_begin + reserved_length;

constexpr types::data_length data_revision_length = 2;
constexpr types::address data_revision_address_begin = reserved_address_end;
constexpr types::address data_revision_address_end =
    data_revision_address_begin + data_revision_length;

constexpr types::data_length lookup_table_tail_length = 2;
constexpr types::address lookup_table_tail_begin = data_revision_address_end;
constexpr types::address lookup_table_tail_end =
    lookup_table_tail_begin + lookup_table_tail_length;

constexpr types::address data_address_begin = lookup_table_tail_end;

}  // namespace addresses
}  // namespace eeprom
