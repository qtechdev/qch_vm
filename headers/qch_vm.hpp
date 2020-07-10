#ifndef __QCH_VM_HPP__
#define __QCH_VM_HPP__
#include <array>
#include <cstdint>
#include <functional>
#include <map>
#include <ostream>
#include <random>
#include <string>

#include "spec.hpp"

namespace qch_vm {
  constexpr uint16_t entry_point = 0x200;
  constexpr uint16_t font_index = 0x50;
  using opcode = uint16_t;

  constexpr std::array<uint8_t, 4> char_lines = {
    0xf0, // ****----
    0x90, // *--*----
    0x10, // ---*----
    0x80  // *-------
  };

  constexpr std::array<uint8_t, 5*16> char_data = {
    0, 1, 1, 1, 0, // 0
    2, 2, 2, 2, 2, // 1
    0, 2, 0, 3, 0, // 2
    0, 2, 0, 2, 0, // 3
    1, 1, 0, 2, 2, // 4
    0, 3, 0, 2, 0, // 5
    0, 3, 0, 1, 0, // 6
    0, 2, 2, 2, 2, // 7
    0, 1, 0, 1, 0, // 8
    0, 1, 0, 2, 2, // 9
    0, 1, 0, 1, 1, // a
    3, 3, 0, 1, 0, // b
    0, 3, 3, 3, 0, // c
    2, 2, 0, 1, 0, // d
    0, 1, 0, 3, 0, // e
    0, 3, 0, 3, 3  // f
  };

  struct machine {
    machine();
    void resize(const uint8_t w, const uint8_t h);

    std::uniform_int_distribution<uint8_t> distribution;
    std::mt19937 engine;

    uint8_t display_width = 1;
    uint8_t display_height = 1;
    std::array<uint8_t, 16> reg = {0};
    std::array<uint8_t, 4096> mem = {0};
    std::vector<uint8_t> gfx = {0}; // resize at run time
    std::array<uint16_t, 16> stack = {0};
    std::array<bool, 16> keys = {0};
    uint16_t pc = entry_point;
    uint16_t I = 0;
    uint8_t sp = 0;
    uint8_t br = 0;
    uint8_t delay_timer = 0;
    uint8_t sound_timer = 0;

    uint16_t program_size;
    bool quit=false;
    bool draw=false;
    bool blocking=false;
    bool halted=false;
    static constexpr std::size_t final_size = 100;
    char final[final_size];
  };

  using fn = std::function<void(machine &m, const qch::instruction &inst)>;

  qch::instruction fetch_instruction(const machine &m);
  fn decode_instruction(const qch::instruction &inst);

  uint8_t sprite_address(const uint8_t index);

  void load_program(machine &m, const std::vector<uint8_t> &program);

  std::string dump_registers(const machine &m, bool ascii=false);
  std::string dump_graphics_data(const machine &m);
  std::string dump_memory(const machine &m);

  void get_key(machine &m);

  void clear(machine &m, const qch::instruction &inst);
  void ret(machine &m, const qch::instruction &inst);
  void jmp(machine &m, const qch::instruction &inst);
  void call(machine &m, const qch::instruction &inst);
  void seq(machine &m, const qch::instruction &inst);
  void sne(machine &m, const qch::instruction &inst);
  void seqr(machine &m, const qch::instruction &inst);
  void mov(machine &m, const qch::instruction &inst);
  void add(machine &m, const qch::instruction &inst);
  void movr(machine &m, const qch::instruction &inst);
  void q_or(machine &m, const qch::instruction &inst);
  void q_and(machine &m, const qch::instruction &inst);
  void q_xor(machine &m, const qch::instruction &inst);
  void addr(machine &m, const qch::instruction &inst);
  void sub(machine &m, const qch::instruction &inst);
  void slr(machine &m, const qch::instruction &inst);
  void rsub(machine &m, const qch::instruction &inst);
  void sll(machine &m, const qch::instruction &inst);
  void sner(machine &m, const qch::instruction &inst);
  void movi(machine &m, const qch::instruction &inst);
  void jmpv(machine &m, const qch::instruction &inst);
  void rand(machine &m, const qch::instruction &inst);
  void draw(machine &m, const qch::instruction &inst);
  void keq(machine &m, const qch::instruction &inst);
  void kne(machine &m, const qch::instruction &inst);
  void std(machine &m, const qch::instruction &inst);
  void key(machine &m, const qch::instruction &inst);
  void ldd(machine &m, const qch::instruction &inst);
  void lds(machine &m, const qch::instruction &inst);
  void addi(machine &m, const qch::instruction &inst);
  void sprite(machine &m, const qch::instruction &inst);
  void bcd(machine &m, const qch::instruction &inst);
  void str(machine &m, const qch::instruction &inst);
  void ldr(machine &m, const qch::instruction &inst);

  void nop(machine &m, const qch::instruction &inst);
  void halt(machine &m, const qch::instruction &inst);

  void panic(machine &m, const qch::instruction &inst);
};

#endif // __QCH_VM_HPP__
