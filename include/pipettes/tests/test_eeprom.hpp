
namespace test_eeprom {

constexpr const size_t BUFFER_SIZE = 5;

class TestI2C {
  public:
    uint8_t stored = 0x0;
    uint8_t DEVICE_ADDRESS = 0x1;
    static constexpr auto BUFFER_SIZE = 1;
    using BufferType = std::array<uint8_t, BUFFER_SIZE>;
    void transmit(uint8_t value) { stored = value; }
    void receive(BufferType& receive) { receive[0] = stored; }
};

};  // namespace test_eeprom
