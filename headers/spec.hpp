#ifndef __QCH_SPEC_HPP__
#define __QCH_SPEC_HPP__
#include <array>
#include <cstdint>
#include <optional>
#include <string>

namespace qch {
  namespace arg_type {
    static constexpr uint8_t R = 0b0001; // Register
    static constexpr uint8_t B = 0b0010; // Byte
    static constexpr uint8_t A = 0b0011; // Address
    static constexpr uint8_t D = 0b0100; // Data
    static constexpr uint8_t N = 0b0101; // Nibble
  };

  enum class args_config {
    Z   = 0x0000,
    R   = 0x0001 | (arg_type::R << 4),
    RR  = 0x0002 | (arg_type::R << 4) | (arg_type::R << 8),
    RB  = 0x0002 | (arg_type::R << 4) | (arg_type::B << 8),
    RRN = 0x0003 | (arg_type::R << 4) | (arg_type::R << 8) | (arg_type::N << 12),
    A   = 0x0001 | (arg_type::A << 4),
    D   = 0x0001 | (arg_type::D << 4),
  };

  struct header {
    uint8_t w;
    uint8_t h;
  };

  static constexpr std::size_t header_size = 16;
  static constexpr header default_header = { 64, 32 };

  static const std::optional<std::vector<uint8_t>> get_header(
    const std::vector<uint8_t> &program
  ) {
    if (program.size() <= header_size) {
      return {};
    }

    return std::vector<uint8_t>(
      program.end() - header_size,
      program.end()
    );
  }

  static const header parse_header(const std::vector<uint8_t> &program) {
    auto raw_header = get_header(program);
    if (!raw_header) {
      return default_header;
    }

    header head = default_header;

    if (((*raw_header)[0] == 0xc8) && ((*raw_header)[1] == 0xc8)) {
      head.w = (*raw_header)[6];
      head.h = (*raw_header)[7];
    }

    return head;
  }

  struct instruction {
    union {
      uint16_t value;
      struct {
        uint8_t lo;
        uint8_t hi;
      };
    };

    uint16_t mask;
    uint16_t data;
    args_config args;
    std::string name;
  };

  static const instruction unknown_instruction = {
    0xdead, 0xbeef, 0x0000, args_config::Z, "unknown instruction"
  };

  static const std::array<instruction, 36> isa = {{
    {0x00e0, 0xffff, 0x0000, args_config::Z, "clear"},
    {0x00ee, 0xffff, 0x0000, args_config::Z, "ret"},
    {0x1000, 0xf000, 0x0000, args_config::A, "jmp"},
    {0x2000, 0xf000, 0x0000, args_config::A, "call"},
    {0x3000, 0xf000, 0x0000, args_config::RB, "seq"},
    {0x4000, 0xf000, 0x0000, args_config::RB, "sne"},
    {0x5000, 0xf000, 0x0000, args_config::RR, "seqr"},
    {0x6000, 0xf000, 0x0000, args_config::RB, "mov"},
    {0x7000, 0xf000, 0x0000, args_config::RB, "add"},
    {0x8000, 0xf00f, 0x0000, args_config::RR, "movr"},
    {0x8001, 0xf00f, 0x0000, args_config::RR, "or"},
    {0x8002, 0xf00f, 0x0000, args_config::RR, "and"},
    {0x8003, 0xf00f, 0x0000, args_config::RR, "xor"},
    {0x8004, 0xf00f, 0x0000, args_config::RR, "addr"},
    {0x8005, 0xf00f, 0x0000, args_config::RR, "sub"},
    {0x8006, 0xf00f, 0x0000, args_config::RR, "slr"},
    {0x8007, 0xf00f, 0x0000, args_config::RR, "rsub"},
    {0x800e, 0xf00f, 0x0000, args_config::RR, "sll"},
    {0x9000, 0xf00f, 0x0000, args_config::RR, "sner"},
    {0xa000, 0xf000, 0x0000, args_config::A, "movi"},
    {0xb000, 0xf000, 0x0000, args_config::A, "jmpv"},
    {0xc000, 0xf000, 0x0000, args_config::RB, "rand"},
    {0xd000, 0xf000, 0x0000, args_config::RRN, "draw"},
    {0xe09e, 0xf0ff, 0x0000, args_config::R, "keq"},
    {0xe0a1, 0xf0ff, 0x0000, args_config::R, "kne"},
    {0xf007, 0xf0ff, 0x0000, args_config::R, "std"},
    {0xf00a, 0xf0ff, 0x0000, args_config::R, "key"},
    {0xf015, 0xf0ff, 0x0000, args_config::R, "ldd"},
    {0xf018, 0xf0ff, 0x0000, args_config::R, "lds"},
    {0xf01e, 0xf0ff, 0x0000, args_config::R, "addi"},
    {0xf029, 0xf0ff, 0x0000, args_config::R, "sprite"},
    {0xf033, 0xf0ff, 0x0000, args_config::R, "bcd"},
    {0xf055, 0xf0ff, 0x0000, args_config::R, "str"},
    {0xf065, 0xf0ff, 0x0000, args_config::R, "ldr"},

    {0x0000, 0xffff, 0x0000, args_config::Z, "nop"},
    {0xffff, 0xffff, 0x0000, args_config::Z, "halt"},
  }};

  static std::string reg_token = "&";
  static std::string data_token = "$ ";
  static std::string label_token = ":";
  static std::string comment_token = "//";

  static constexpr uint8_t get_r(const instruction &inst) {
    return (inst.data & 0x0f00) >> 8;
  }

  static constexpr uint16_t get_a(const instruction &inst) {
    return inst.data & 0x0fff;
  }

  static constexpr std::array<uint8_t, 2> get_rr(const instruction &inst) {
    uint8_t x = (inst.data & 0x0f00) >> 8;
    uint8_t y = (inst.data & 0x00f0) >> 4;

    return {x, y};
  }

  static constexpr std::array<uint8_t, 2> get_rb(const instruction &inst) {
    uint8_t x = (inst.data & 0x0f00) >> 8;
    uint8_t b = (inst.data & 0x00ff);

    return {x, b};
  }

  static constexpr std::array<uint8_t, 3> get_rrn(const instruction &inst) {
    uint8_t x = (inst.data & 0x0f00) >> 8;
    uint8_t y = (inst.data & 0x00f0) >> 4;
    uint8_t n = (inst.data & 0x000f);

    return {x, y, n};
  }
};

#endif // __QCH_SPEC_HPP__
