#pragma once

namespace bit_utils {

static const int byte_mask = 0xFF;

/**
 * Convert a span of bytes into a single integer. The first index being the most
 * significant byte. Big endian.
 * @tparam OutputType The type of the result. Must be an integer.
 * @tparam InputType The type contained in the span. Must be an integer.
 * @param input A span of integers
 * @param output The output parameter.
 */
template <typename OutputType, typename InputType>
requires std::is_integral_v<OutputType> &&std::is_integral_v<InputType> void
bytes_to_int(const std::span<InputType> &input, OutputType &output) {
    output = 0;
    for (auto byte : input) {
        output <<= 8;
        output |= (byte & byte_mask);
    }
}

/**
 * Write an integer into an iterator. Big Endian.
 * @tparam OutputIter An iterator to write into.
 * @tparam InputType The type written into the iterator. Must be an integer.
 * @param input integer
 * @param output An iterator
 * @returns iterator at end of written bytes
 */
template <typename InputType, typename OutputIter>
requires std::forward_iterator<OutputIter> &&std::is_integral_v<InputType> auto
int_to_bytes(InputType input, OutputIter iter) -> OutputIter {
    for (int x = sizeof(input) - 1; x >= 0; x--) {
        *iter++ = (input >> (x * 8)) & byte_mask;
    }
    return iter;
}

}  // namespace bit_utils
