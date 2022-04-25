namespace sensors {
namespace registers {

template <typename Reg>
concept WritableRegister = requires() {
    {Reg::writable};
};

template <typename Reg>
concept ReadableRegister = requires() {
    {Reg::readable};
};
};  // namespace registers
};  // namespace sensors
