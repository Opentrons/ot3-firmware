#pragma once


namespace bit_utils {
    static const int byte_mask = 0xFF;

    /**
     * Convert a span of bytes into a single integer. The first index being the most significant
     * byte. Big endian.
     * @tparam OutputType The type of the result. Must be an integer.
     * @tparam InputType The type contained in the span. Must be an integer.
     * @param input A span of integers
     * @param output The output parameter.
     */
    template<typename OutputType, typename InputType>
    requires std::is_integral_v<OutputType> && std::is_integral_v<InputType>
    void bytes_to_int(const std::span<InputType> &input, OutputType &output) {
        output = 0;
        for (auto byte: input) {
            output <<= 8;
            output |= (byte & byte_mask);
        }
    }
}
