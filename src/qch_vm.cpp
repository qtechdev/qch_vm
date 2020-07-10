#include <array>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <ostream>
#include <sstream>
#include <string>

#include "spec.hpp"

#include "qch_vm.hpp"

static std::map<uint16_t, qch_vm::fn> fn_lookup = {
  {0x00e0, qch_vm::clear},
  {0x00ee, qch_vm::ret},
  {0x1000, qch_vm::jmp},
  {0x2000, qch_vm::call},
  {0x3000, qch_vm::seq},
  {0x4000, qch_vm::sne},
  {0x5000, qch_vm::seqr},
  {0x6000, qch_vm::mov},
  {0x7000, qch_vm::add},
  {0x8000, qch_vm::movr},
  {0x8001, qch_vm::q_or},
  {0x8002, qch_vm::q_and},
  {0x8003, qch_vm::q_xor},
  {0x8004, qch_vm::addr},
  {0x8005, qch_vm::sub},
  {0x8006, qch_vm::slr},
  {0x8007, qch_vm::rsub},
  {0x800e, qch_vm::sll},
  {0x9000, qch_vm::sner},
  {0xa000, qch_vm::movi},
  {0xb000, qch_vm::jmpv},
  {0xc000, qch_vm::rand},
  {0xd000, qch_vm::draw},
  {0xe09e, qch_vm::keq},
  {0xe0a1, qch_vm::kne},
  {0xf007, qch_vm::std},
  {0xf00a, qch_vm::key},
  {0xf015, qch_vm::ldd},
  {0xf018, qch_vm::lds},
  {0xf01e, qch_vm::addi},
  {0xf029, qch_vm::sprite},
  {0xf033, qch_vm::bcd},
  {0xf055, qch_vm::str},
  {0xf065, qch_vm::ldr},
  {0x0000, qch_vm::nop},
  {0xffff, qch_vm::halt},
};

qch_vm::machine::machine() {
  for (int i = 0; i < char_data.size(); i++) {
    mem[font_index + i] = char_lines[char_data[i]];
  }

  engine.seed(std::random_device{}());
};

void qch_vm::machine::resize(const uint8_t w, const uint8_t h) {
  display_width = w;
  display_height = h;

  reg = {
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    w, h, 0, 0
  };

  gfx.resize(w*h);
}

qch::instruction modify(const qch::instruction &inst, const uint16_t data) {
  qch::instruction new_inst = inst;
  new_inst.data = data;

  return new_inst;
}

qch::instruction qch_vm::fetch_instruction(const machine &m) {
  uint16_t x = (m.mem.at(m.pc) << 8) | m.mem.at(m.pc + 1);

  for (const auto &inst_ref : qch::isa) {
    if (inst_ref.value == (x & inst_ref.mask)) {
      qch::instruction inst = inst_ref;
      inst.data = x;

      return inst;
    }
  }

  return qch::unknown_instruction;
}

qch_vm::fn qch_vm::decode_instruction(const qch::instruction &inst) {
  return fn_lookup.at(inst.value & inst.mask);
}

uint8_t qch_vm::sprite_address(const uint8_t index) {
  return font_index + (index * 5);
}

void qch_vm::load_program(machine &m, const std::vector<uint8_t> &program) {
  std::copy(program.begin(), program.end(), &m.mem[entry_point]);
  m.program_size = program.size();

  auto [w, h] = qch::parse_header(program);

  m.resize(w, h);
}

std::string qch_vm::dump_registers(const machine &m, bool ascii) {
  std::stringstream ss;
  char buf[12];
  ss << "| ";
  for (int i = 0; i < 8; i++) {
    snprintf(buf, 5, "  V%x", i);
    ss << buf << " | ";
  }
  ss << "\n| ";
  for (int i = 0; i < 8; i++) {
    snprintf(buf, 5, (ascii ? "%4c" : "%#04x"), m.reg[i]);
    ss << buf << " | ";
  }
  ss << "\n\n| ";

  for (int i = 8; i < 16; i++) {
    snprintf(buf, 5, "  V%x", i);
    ss << buf << " | ";
  }
  ss << "\n| ";
  for (int i = 8; i < 16; i++) {
    snprintf(buf, 5, (ascii ? "%4c" : "%#04x"), m.reg[i]);
    ss << buf << " | ";
  }
  ss << "\n\n| ";


  ss << "          I |          pc |   sp |   dt |   st |\n| ";
  snprintf(buf, 12, "      %#05x", m.I);
  ss << buf << " | ";
  snprintf(buf, 12, "      %#05x", m.pc);
  ss << buf << " | ";
  snprintf(buf, 5, "%#04x", m.sp);
  ss << buf << " | ";
  snprintf(buf, 5, "%#04x", m.delay_timer);
  ss << buf << " | ";
  snprintf(buf, 5, "%#04x", m.sound_timer);
  ss << buf << " | ";

  return ss.str();
}

std::string qch_vm::dump_memory(const machine &m) {
  std::stringstream ss;

  for (int i = 0; i < 4096; i += 16) {
    std::stringstream line;
    char buf[5];
    snprintf(buf, 5, "%04x", i);

    line << buf << " ";

    for (int j = 0; j < 16; j++) {
      snprintf(buf, 3, "%02x", m.mem[i + j]);
      line << buf << " ";
    }

    line << " : ";

    for (int j = 0; j < 16; j++) {
      char c = m.mem[i + j];
      snprintf(buf, 2, ((c >= ' ' && c <= '~') ? "%c" : "."), c);
      line << buf;
    }

    ss << line.str() << "\n";
  }

  return ss.str();
}

std::string qch_vm::dump_graphics_data(const machine &m) {
  std::stringstream ss;

  for (int i = 0; i < 32*64; i += 64) {
    std::stringstream line;
    char buf[2];

    for (int j = 0; j < 64; j++) {
      snprintf(buf, 2, "%x", m.gfx[i + j]);
      line << buf;
    }

    ss << line.str() << "\n";
  }

  return ss.str();
}

void qch_vm::get_key(machine &m) {
  for (int k = 0; k < m.keys.size(); k++) {
    if (m.keys[k]) {
      m.reg[m.br] = k;
      m.blocking = false;
      return;
    }
  }
}

void qch_vm::clear(machine &m, const qch::instruction &inst) {
  std::fill(m.gfx.begin(), m.gfx.end(), 0);
  m.draw = true;
  m.pc += 2;
}

void qch_vm::ret(machine &m, const qch::instruction &inst) {
  m.pc = m.stack[--m.sp];
}

void qch_vm::jmp(machine &m, const qch::instruction &inst) {
  auto addr = get_a(inst);

  m.pc = addr;
}

void qch_vm::call(machine &m, const qch::instruction &inst) {
  auto addr = get_a(inst);

  m.stack[m.sp++] = m.pc + 2;
  m.pc = addr;
}

void qch_vm::seq(machine &m, const qch::instruction &inst) {
  auto [r, v] = get_rb(inst);

  if (m.reg[r] == v) {
    m.pc += 2;
  }
  m.pc += 2;
}

void qch_vm::sne(machine &m, const qch::instruction &inst) {
  auto [r, v] = get_rb(inst);

  if (m.reg[r] != v) {
    m.pc += 2;
  }
  m.pc += 2;
}

void qch_vm::seqr(machine &m, const qch::instruction &inst) {
  auto [x, y] = get_rr(inst);

  if (m.reg[x] == m.reg[y]) {
    m.pc += 2;
  }
  m.pc += 2;
}

void qch_vm::mov(qch_vm::machine &m, const qch::instruction &inst) {
  auto [r, v] = get_rb(inst);

  m.reg[r] = v;
  m.pc += 2;
}

void qch_vm::add(machine &m, const qch::instruction &inst) {
  auto [r, v] = get_rb(inst);

  m.reg[r] += v;
  m.pc += 2;
}

void qch_vm::movr(machine &m, const qch::instruction &inst) {
  auto [x, y] = get_rr(inst);

  m.reg[x] = m.reg[y];
  m.pc += 2;
}

void qch_vm::q_or(machine &m, const qch::instruction &inst) {
  auto [x, y] = get_rr(inst);

  m.reg[x] |= m.reg[y];
  m.pc += 2;
}

void qch_vm::q_and(machine &m, const qch::instruction &inst) {
  auto [x, y] = get_rr(inst);

  m.reg[x] &= m.reg[y];
  m.pc += 2;
}

void qch_vm::q_xor(machine &m, const qch::instruction &inst) {
  auto [x, y] = get_rr(inst);

  m.reg[x] ^= m.reg[y];
  m.pc += 2;
}

void qch_vm::addr(machine &m, const qch::instruction &inst) {
  auto [x, y] = get_rr(inst);

  uint16_t tmp = m.reg[x] + m.reg[y];
  // set flag if carry (i.e. 9th bit is set)
  m.reg[0xf] = (tmp & 0x100) >> 8;
  m.reg[x] = tmp & 0xff;
  m.pc += 2;
}

void qch_vm::sub(machine &m, const qch::instruction &inst) {
  auto [x, y] = get_rr(inst);

  uint16_t tmp = m.reg[x] - m.reg[y];
  // set flag if borrow (i.e. 9th bit is not set)
  m.reg[0xf] =  !((tmp & 0x100) >> 8);
  m.reg[x] = tmp & 0xff;
  m.pc += 2;
}

void qch_vm::slr(machine &m, const qch::instruction &inst) {
  auto [x, y] = get_rr(inst);

  m.reg[0xf] =  m.reg[x] & 0x01;
  m.reg[x] >>= 1;
  m.pc += 2;
}

void qch_vm::rsub(machine &m, const qch::instruction &inst) {
  auto [x, y] = get_rr(inst);

  uint16_t tmp = m.reg[y] - m.reg[x];
  // set flag if borrow (i.e. 9th bit is not set)
  m.reg[0xf] =  !((tmp & 0x100) >> 8);
  m.reg[x] = tmp & 0xff;
}

void qch_vm::sll(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);

  m.reg[0xf] =  (m.reg[x] & 0x70) >> 7;
  m.reg[x] <<= 1;
  m.pc += 2;
}

void qch_vm::sner(machine &m, const qch::instruction &inst) {
  auto [x, y] = get_rr(inst);

  if (m.reg[x] != m.reg[y]) {
    m.pc += 2;
  }
  m.pc += 2;
}

void qch_vm::movi(machine &m, const qch::instruction &inst) {
  auto addr = get_a(inst);

  m.I = addr;
  m.pc += 2;
}

void qch_vm::jmpv(machine &m, const qch::instruction &inst) {
  auto addr = get_a(inst);

  m.pc = addr + m.reg[0];
}

void qch_vm::rand(machine &m, const qch::instruction &inst) {
  auto [r, v] = get_rb(inst);

  uint8_t rng = m.distribution(m.engine);
  m.reg[r] = (rng % 256) & v;
  m.pc += 2;
}

void qch_vm::draw(machine &m, const qch::instruction &inst) {
  auto [x, y, n] = get_rrn(inst);

  m.reg[0xf] = 0;

  for (int row_offset = 0; row_offset < n; row_offset++) {
    uint8_t data_8_wide = m.mem[m.I + row_offset];

    for (int px = 0; px < 8; px++) {
      if ((data_8_wide & (0x80 >> px)) != 0) {
        std::size_t gfx_row = row_offset + m.reg[y];
        std::size_t gfx_col = px + m.reg[x];
        std::size_t gfx_index = (gfx_row * m.display_width) + gfx_col;
        if (m.gfx[gfx_index] == 1) {
          m.reg[0xf] = 1;
        }
        m.gfx[gfx_index] ^= 1;
      }
    }
  }

  m.draw = true;
  m.pc +=  2;
}

void qch_vm::keq(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);
    if (m.keys[m.reg[x]]) {
    m.pc += 2;
  }

  m.pc += 2;
}
void qch_vm::kne(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);
    if (m.keys[m.reg[x]]) {
    m.pc += 2;
  }

  m.pc += 2;
}

void qch_vm::std(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);

  m.reg[x] = m.delay_timer;
  m.pc += 2;
}

void qch_vm::key(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);

  m.blocking = true;
  m.br = x;
  m.pc += 2;
}

void qch_vm::ldd(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);

  m.delay_timer = m.reg[x];
  m.pc += 2;
}

void qch_vm::lds(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);

  m.sound_timer = m.reg[x];
  m.pc += 2;
}

void qch_vm::addi(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);

  uint16_t tmp = m.reg[x] + m.I;
  // set flag if carry (i.e. 9th bit is set)
  m.reg[0xf] = (tmp & 0x100) >> 8;
  m.I = tmp & 0xff;
  m.pc += 2;
}

void qch_vm::sprite(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);

  m.I = sprite_address(m.reg[x]);
  m.pc += 2;
}

void qch_vm::bcd(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);

  m.mem[m.I] = m.reg[x] / 100;
  m.mem[m.I + 1] = (m.reg[x] / 10) % 10;
  m.mem[m.I + 2] = m.reg[x] % 10;
  m.pc += 2;
}

void qch_vm::str(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);

  for (int i = 0; i < x; i++) {
    m.mem[m.I + i] = m.reg[i];
  }
  m.pc += 2;
}

void qch_vm::ldr(machine &m, const qch::instruction &inst) {
  auto x = get_r(inst);

  for (int i = 0; i < x; i++) {
    m.reg[i] = m.mem[m.I + i];
  }
  m.pc += 2;
}

void qch_vm::nop(machine &m, const qch::instruction &inst) {
  m.pc += 2;
}

void qch_vm::halt(machine &m, const qch::instruction &inst) {
  snprintf(
    m.final, m.final_size,
    "Halting Machine @ %#06x", m.pc
  );

  m.halted = true;
}

void qch_vm::panic(machine &m, const qch::instruction &inst) {
  snprintf(
    m.final, m.final_size,
    "Unknown Instruction! [%#06x] @ %#06x", inst.data, m.pc
  );

  m.quit = true;
}
