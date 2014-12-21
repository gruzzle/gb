#include "cpu.h"

uint16_t pc, sp;
uint8_t registers[8];
bool prefixCB;
bool interruptsEnabled;

const uint8_t opcodeLength[] = {
  1,3,1,1,1,1,2,1,3,1,1,1,1,1,2,1,
  2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
  2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
  2,3,1,1,1,1,2,1,2,1,1,1,1,1,2,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,3,3,3,1,2,1,1,1,3,1,3,3,2,1,
  1,1,3,0,3,1,2,1,1,1,3,0,3,0,2,1,
  2,1,2,0,0,1,2,1,2,1,3,0,0,0,2,1,
  2,1,2,1,0,1,2,1,2,1,3,1,0,0,2,1
};

void initializeCPU() {
  pc = 0x0;
  //  sp = 0xFFFE;
  prefixCB = false;
  interruptsEnabled = true;
}

void mainLoop() {
  bool running = true;
  int count = 100;
  
  uint8_t opcode;
  while(running && count > 0) {
    opcode = readNextByte();
    executeOpcode(opcode);
    printRegisters();
    printf("\n");
    count--;
  }
}

uint8_t readNextByte() {
  uint8_t byte = readMemory(pc);
  pc++;
  return byte;
}

uint8_t getHighByte(uint16_t value) {
  return value >> 8;
}

uint8_t getLowByte(uint16_t value) {
  return value & 0xFF;
}

uint16_t combineBytes(uint8_t high, uint8_t low) {
  return (high << 8) | low;
}

uint8_t getBit(uint8_t value, uint8_t n) {
  return (value & (uint8_t) pow(2,n)) >> n;
}

const char *byteToBinary(uint8_t x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 128; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

void print16(uint16_t value) {
  printf("%#06X\n", value);
}

void printRegisters() {
  printf("pc: 0x%04X, sp: 0x%04X | A: 0x%02X, B: 0x%02X, C: 0x%02X, D: 0x%02X, E: 0x%02X, H: 0x%02X, L: 0x%02X, F: 0x%s\n",
	 pc, sp, readReg(REG_A), readReg(REG_B), readReg(REG_C), readReg(REG_D), readReg(REG_E), readReg(REG_H), readReg(REG_L), byteToBinary(readReg(REG_F)));
}

uint8_t pop() {
  uint8_t value = readMemory(sp);
  sp++;
  return value;
}

uint16_t popWord() {
  uint8_t low = readMemory(sp);
  sp++;
  uint8_t high = readMemory(sp);
  sp++;
  return combineBytes(high, low);
}

void push(uint8_t value) {
  sp--;
  writeMemory(sp, value);
}

void pushWord(uint16_t value) {
  sp--;
  writeMemory(sp, getHighByte(value));
  sp--;
  writeMemory(sp, getLowByte(value));
}

uint16_t readReg(registerName reg) {
  switch (reg) {
  case(REG_AF):
    return combineBytes(registers[REG_A], registers[REG_F]);    
  case(REG_BC):
    return combineBytes(registers[REG_B], registers[REG_C]);
  case(REG_DE):
    return combineBytes(registers[REG_D], registers[REG_E]);
  case(REG_HL):
    return combineBytes(registers[REG_H], registers[REG_L]);
  default:
    return registers[reg];
  }
}

void writeReg(registerName reg, uint16_t value) {
  switch(reg) {
  case(REG_AF):
    registers[REG_A] = getHighByte(value);
    registers[REG_F] = getLowByte(value);  
    break;
  case(REG_BC):
    registers[REG_B] = getHighByte(value);
    registers[REG_C] = getLowByte(value);  
    break;
  case(REG_DE):
    registers[REG_D] = getHighByte(value);
    registers[REG_E] = getLowByte(value);
    break;
  case(REG_HL):
    registers[REG_H] = getHighByte(value);
    registers[REG_L] = getLowByte(value);
    break;
  default:
    registers[reg] = (uint8_t) value;
  }
}

uint8_t getFlag(char flag) {
  uint8_t flags = readReg(REG_F);
  switch(flag) {
  case 'Z':
    return flags >> 7;    
  case 'N':
    return (flags & 0x40) >> 6;
  case 'H':
    return (flags & 0x20) >> 5;    
  case 'C':
    return (flags & 0x10) >> 4;
  default:
    printf("Error: tried to check unknown flag: %c\n", flag);
    exit(1);
  }
}

void setFlag(char flag, bool value) {
  uint8_t bit;
  switch(flag) {
  case 'Z':
    bit = 7;
    break;
  case 'N':
    bit = 6;
    break;
  case 'H':
    bit = 5;
    break;
  case 'C':
    bit = 4;
    break;
  default:
    printf("Error: tried to set unknown flag: %c\n", flag);
    exit(1);
  }

  if (value) {
    writeReg(REG_F, readReg(REG_F) | (1 << bit));
  }
  else {
    writeReg(REG_F, readReg(REG_F) & ~(1 << bit));
  }

}

void executeOpcode(uint8_t opcode) {  
  uint8_t byteA = 0;
  uint8_t byteB = 0;

  if (!prefixCB) { // normal opcode
    
    if (opcodeLength[opcode] >= 2) {
      byteA = readNextByte();
    }
    if (opcodeLength[opcode] >= 3) {
      byteB = readNextByte();
    }

    printf("opcode: 0x%02X 0x%02X 0x%02X\n", opcode, byteA, byteB);    
  
    switch(opcode) {
    case 0x00:
      NOP();
      break;    
    case 0x01:
      LD_word(REG_BC, combineBytes(byteA, byteB));
      break;
    case 0x02:
      LD_mem(readReg(REG_BC), readReg(REG_A));
      break;
    case 0x03:
      INC(REG_BC);
      break;
    case 0x04:
      INC(REG_B);
      break;
    case 0x05:
      DEC(REG_B);
      break;
    case 0x06:
      LD(REG_B, byteA);
      break;
    case 0x07:
      RLCA();
      break;
    case 0x08:
      LD_mem_word(combineBytes(byteA, byteB), sp);
      break;
    case 0x09:
      ADD_word(readReg(REG_BC));
      break;
    case 0x0A:
      LD(REG_A, readMemory(readReg(REG_BC)));
      break;
    case 0x0B:
      DEC(REG_BC);
      break;
    case 0x0C:
      INC(REG_C);
      break;
    case 0x0D:
      DEC(REG_C);
      break;
    case 0x0E:
      LD(REG_C, byteA);
      break;
    case 0x0F:
      RRCA();
      break;
    case 0x10:
      STOP();
      break;
    case 0x11:
      LD_word(REG_DE, combineBytes(byteA, byteB));
      break;
    case 0x12:
      LD_mem(readReg(REG_DE), readReg(REG_A));
      break;
    case 0x13:
      INC(REG_DE);
      break;
    case 0x14:
      INC(REG_D);
      break;
    case 0x15:
      DEC(REG_D);
      break;
    case 0x16:
      LD(REG_D, byteA);
      break;
    case 0x17:
      RLA();
      break;
    case 0x18:
      JR(byteA);
      break;
    case 0x19:
      ADD_word(readReg(REG_DE));
      break;
    case 0x1A:
      LD(REG_A, readMemory(readReg(REG_DE)));
      break;
    case 0x1B:
      DEC(REG_DE);
      break;
    case 0x1C:
      INC(REG_E);
      break;
    case 0x1D:
      DEC(REG_E);
      break;
    case 0x1E:
      LD(REG_E, byteA);
      break;
    case 0x1F:
      RRA();
      break;
    case 0x20:
      JR_NZ(byteA);
      break;
    case 0x21:
      LD_word(REG_HL, combineBytes(byteA, byteB));
      break;
    case 0x22:
      LD_mem(readReg(REG_HL), readReg(REG_A));
      INC(REG_HL);
      break;
    case 0x23:
      INC(REG_HL);
      break;
    case 0x24:
      INC(REG_H);
      break;
    case 0x25:
      DEC(REG_H);
      break;
    case 0x26:
      LD(REG_H, byteA);
      break;
    case 0x28:
      DAA();
      break;
    case 0x29:
      JR_Z(byteA);
      break;
    case 0x2A:
      LD(REG_A, readMemory(readReg(REG_HL)));
      INC(REG_HL);
      break;
    case 0x2B:
      DEC(REG_HL);
      break;
    case 0x2C:
      INC(REG_L);
      break;
    case 0x2D:
      DEC(REG_L);
      break;
    case 0x2E:
      LD(REG_L, byteA);
      break;
    case 0x2F:
      CPL();
      break;
    case 0x30:
      JR_NC(byteA);
      break;
    case 0x31:
      LD_sp(combineBytes(byteA, byteB));
      break;
    case 0x32:
      LD_mem(readReg(REG_HL), readReg(REG_A));
      DEC(REG_HL);
      break;
    case 0x33:
      INC_sp();
      break;
    case 0x34:
      INC_mem(readReg(REG_HL));
      break;
    case 0x35:
      DEC_mem(readReg(REG_HL));
      break;
    case 0x36:
      LD_mem(readReg(REG_HL), byteA);
      break;
    case 0x37:
      SCF();
      break;
    case 0x38:
      JR_C(byteA);
      break;
    case 0x39:
      ADD_word(sp);
      break;
    case 0x3A:
      LD(REG_A, readMemory(readReg(REG_HL)));
      INC(REG_HL);
      break;
    case 0x3B:
      DEC_sp();
      break;
    case 0x3C:
      INC(REG_L);
      break;
    case 0x3D:
      DEC(REG_A);
      break;
    case 0x3E:
      LD(REG_A, byteA);
      break;
    case 0x3F:
      CCF();
      break;
    case 0x40:
      LD(REG_B, readReg(REG_B));
      break;
    case 0x41:
      LD(REG_B, readReg(REG_C));
      break;
    case 0x42:
      LD(REG_B, readReg(REG_D));
      break;
    case 0x43:
      LD(REG_B, readReg(REG_E));
      break;
    case 0x44:
      LD(REG_B, readReg(REG_H));
      break;
    case 0x45:
      LD(REG_B, readReg(REG_L));
      break;
    case 0x46:
      LD(REG_B, readMemory(readReg(REG_HL)));
      break;
    case 0x47:
      LD(REG_B, readReg(REG_A));
      break;
    case 0x48:
      LD(REG_C, readReg(REG_B));
      break;
    case 0x49:
      LD(REG_C, readReg(REG_C));
      break;
    case 0x4A:
      LD(REG_C, readReg(REG_D));
      break;
    case 0x4B:
      LD(REG_C, readReg(REG_E));
      break;
    case 0x4C:
      LD(REG_C, readReg(REG_H));
      break;
    case 0x4D:
      LD(REG_C, readReg(REG_L));
      break;
    case 0x4E:
      LD(REG_C, readMemory(readReg(REG_HL)));
      break;
    case 0x4F:
      LD(REG_C, readReg(REG_A));
      break;
    case 0x50:
      LD(REG_D, readReg(REG_B));
      break;
    case 0x51:
      LD(REG_D, readReg(REG_C));
      break;
    case 0x52:
      LD(REG_D, readReg(REG_D));
      break;
    case 0x53:
      LD(REG_D, readReg(REG_E));
      break;
    case 0x54:
      LD(REG_D, readReg(REG_H));
      break;
    case 0x55:
      LD(REG_D, readReg(REG_L));
      break;
    case 0x56:
      LD(REG_D, readMemory(readReg(REG_HL)));
      break;
    case 0x57:
      LD(REG_D, readReg(REG_A));
      break;
    case 0x58:
      LD(REG_E, readReg(REG_B));
      break;
    case 0x59:
      LD(REG_E, readReg(REG_C));
      break;
    case 0x5A:
      LD(REG_E, readReg(REG_D));
      break;
    case 0x5B:
      LD(REG_E, readReg(REG_E));
      break;
    case 0x5C:
      LD(REG_E, readReg(REG_H));
      break;
    case 0x5D:
      LD(REG_E, readReg(REG_L));
      break;
    case 0x5E:
      LD(REG_E, readMemory(readReg(REG_HL)));
      break;
    case 0x5F:
      LD(REG_E, readReg(REG_A));
      break;
    case 0x60:
      LD(REG_H, readReg(REG_B));
      break;
    case 0x61:
      LD(REG_H, readReg(REG_C));
      break;
    case 0x62:
      LD(REG_H, readReg(REG_D));
      break;
    case 0x63:
      LD(REG_H, readReg(REG_E));
      break;
    case 0x64:
      LD(REG_H, readReg(REG_H));
      break;
    case 0x65:
      LD(REG_H, readReg(REG_L));
      break;
    case 0x66:
      LD(REG_H, readMemory(readReg(REG_HL)));
      break;
    case 0x67:
      LD(REG_H, readReg(REG_A));
      break;
    case 0x68:
      LD(REG_L, readReg(REG_B));
      break;
    case 0x69:
      LD(REG_L, readReg(REG_C));
      break;
    case 0x6A:
      LD(REG_L, readReg(REG_D));
      break;
    case 0x6B:
      LD(REG_L, readReg(REG_E));
      break;
    case 0x6C:
      LD(REG_L, readReg(REG_H));
      break;
    case 0x6D:
      LD(REG_L, readReg(REG_L));
      break;
    case 0x6E:
      LD(REG_L, readMemory(readReg(REG_HL)));
      break;
    case 0x6F:
      LD(REG_L, readReg(REG_A));
      break;
    case 0x70:
      LD_mem(readReg(REG_HL), readReg(REG_B));
      break;
    case 0x71:
      LD_mem(readReg(REG_HL), readReg(REG_C));
      break;
    case 0x72:
      LD_mem(readReg(REG_HL), readReg(REG_D));
      break;
    case 0x73:
      LD_mem(readReg(REG_HL), readReg(REG_E));
      break;
    case 0x74:
      LD_mem(readReg(REG_HL), readReg(REG_H));
      break;
    case 0x75:
      LD_mem(readReg(REG_HL), readReg(REG_L));
      break;
    case 0x76:
      HALT();
      break;
    case 0x77:
      LD_mem(readReg(REG_HL), readReg(REG_A));
      break;
    case 0x78:
      LD(REG_A, readReg(REG_B));
      break;
    case 0x79:
      LD(REG_A, readReg(REG_C));
      break;
    case 0x7A:
      LD(REG_A, readReg(REG_D));
      break;
    case 0x7B:
      LD(REG_A, readReg(REG_E));
      break;
    case 0x7C:
      LD(REG_A, readReg(REG_H));
      break;
    case 0x7D:
      LD(REG_A, readReg(REG_L));
      break;
    case 0x7E:
      LD(REG_A, readMemory(readReg(REG_HL)));
      break;
    case 0x7F:
      LD(REG_A, readReg(REG_A));
      break;
    case 0x80:
      ADD(readReg(REG_B));
      break;
    case 0x81:
      ADD(readReg(REG_C));
      break;
    case 0x82:
      ADD(readReg(REG_D));
      break;
    case 0x83:
      ADD(readReg(REG_E));
      break;
    case 0x84:
      ADD(readReg(REG_H));
      break;
    case 0x85:
      ADD(readReg(REG_L));
      break;
    case 0x86:
      ADD(readMemory(readReg(REG_HL)));
      break;
    case 0x87:
      ADD(readReg(REG_A));
      break;
    case 0x88:
      ADC(readReg(REG_B));
      break;
    case 0x89:
      ADC(readReg(REG_C));
      break;
    case 0x8A:
      ADC(readReg(REG_D));
      break;
    case 0x8B:
      ADC(readReg(REG_E));
      break;
    case 0x8C:
      ADC(readReg(REG_H));
      break;
    case 0x8D:
      ADC(readReg(REG_L));
      break;
    case 0x8E:
      ADC(readMemory(readReg(REG_HL)));
      break;
    case 0x8F:
      ADC(readReg(REG_A));
      break;
    case 0x90:
      SUB(readReg(REG_B));
      break;
    case 0x91:
      SUB(readReg(REG_C));
      break;
    case 0x92:
      SUB(readReg(REG_D));
      break;
    case 0x93:
      SUB(readReg(REG_E));
      break;
    case 0x94:
      SUB(readReg(REG_H));
      break;
    case 0x95:
      SUB(readReg(REG_L));
      break;
    case 0x96:
      SUB(readMemory(readReg(REG_HL)));
      break;
    case 0x97:
      SUB(readReg(REG_A));
      break;
    case 0x98:
      SBC(readReg(REG_B));
      break;
    case 0x99:
      SBC(readReg(REG_C));
      break;
    case 0x9A:
      SBC(readReg(REG_D));
      break;
    case 0x9B:
      SBC(readReg(REG_E));
      break;
    case 0x9C:
      SBC(readReg(REG_H));
      break;
    case 0x9D:
      SBC(readReg(REG_L));
      break;
    case 0x9E:
      SBC(readMemory(readReg(REG_HL)));
      break;
    case 0x9F:
      SBC(readReg(REG_A));
      break;
    case 0xA0:
      AND(readReg(REG_B));
      break;
    case 0xA1:
      AND(readReg(REG_C));
      break;
    case 0xA2:
      AND(readReg(REG_D));
      break;
    case 0xA3:
      AND(readReg(REG_E));
      break;
    case 0xA4:
      AND(readReg(REG_H));
      break;
    case 0xA5:
      AND(readReg(REG_L));
      break;
    case 0xA6:
      AND(readMemory(readReg(REG_HL)));
      break;
    case 0xA7:
      AND(readReg(REG_A));
      break;
    case 0xA8:
      XOR(readReg(REG_B));
      break;
    case 0xA9:
      XOR(readReg(REG_C));
      break;
    case 0xAA:
      XOR(readReg(REG_D));
      break;
    case 0xAB:
      XOR(readReg(REG_E));
      break;
    case 0xAC:
      XOR(readReg(REG_H));
      break;
    case 0xAD:
      XOR(readReg(REG_L));
      break;
    case 0xAE:
      XOR(readMemory(readReg(REG_HL)));
      break;
    case 0xAF:
      XOR(readReg(REG_A));
      break;
    case 0xB0:
      OR(readReg(REG_B));
      break;
    case 0xB1:
      OR(readReg(REG_C));
      break;
    case 0xB2:
      OR(readReg(REG_D));
      break;
    case 0xB3:
      OR(readReg(REG_E));
      break;
    case 0xB4:
      OR(readReg(REG_H));
      break;
    case 0xB5:
      OR(readReg(REG_L));
      break;
    case 0xB6:
      OR(readMemory(readReg(REG_HL)));
      break;
    case 0xB7:
      OR(readReg(REG_A));
      break;
    case 0xB8:
      CP(readReg(REG_B));
      break;
    case 0xB9:
      CP(readReg(REG_C));
      break;
    case 0xBA:
      CP(readReg(REG_D));
      break;
    case 0xBB:
      CP(readReg(REG_E));
      break;
    case 0xBC:
      CP(readReg(REG_H));
      break;
    case 0xBD:
      CP(readReg(REG_L));
      break;
    case 0xBE:
      CP(readMemory(readReg(REG_HL)));
      break;
    case 0xBF:
      CP(readReg(REG_A));
      break;
    case 0xC0:
      RET_NZ();
      break;
    case 0xC1:
      POP(REG_BC);
      break;
    case 0XC2:
      JP_NZ(combineBytes(byteA, byteB));
      break;
    case 0XC3:
      JP(combineBytes(byteA, byteB));
      break;
    case 0xC4:
      CALL_NZ(combineBytes(byteA, byteB));
      break;
    case 0xC5:
      PUSH(REG_BC);
      break;
    case 0xC6:
      ADD(byteA);
      break;
    case 0xC7:
      RST(0x00);
      break;
    case 0xC8:
      RET_Z();
      break;
    case 0xC9:
      RET();
      break;
    case 0xCA:
      JP_Z(combineBytes(byteA, byteB));;
      break;
    case 0xCB:
      prefixCB = true;
      break;
    case 0xCC:
      CALL_Z(combineBytes(byteA, byteB));
      break;
    case 0xCD:
      CALL(combineBytes(byteA, byteB));
      break;
    case 0xCE:
      ADC(byteA);
      break;
    case 0xCF:
      RST(0x08);
      break;
    case 0xD0:
      RET_NC();
      break;
    case 0XD1:
      POP(readReg(REG_DE));
      break;
    case 0xD2:
      JP_NC(combineBytes(byteA, byteB));
      break;
    case 0xD4:
      CALL_NC(combineBytes(byteA, byteB));
      break;
    case 0xD5:
      PUSH(readReg(REG_DE));
      break;
    case 0xD6:
      SUB(byteA);
      break;
    case 0xD7:
      RST(0x10);
      break;
    case 0xD8:
      RET_C();
      break;
    case 0xD9:
      RETI();
      break;
    case 0xDA:
      JP_C(combineBytes(byteA, byteB));
      break;
    case 0xDC:
      CALL_C(combineBytes(byteA, byteB));
      break;
    case 0XDE:
      SBC(byteA);
      break;
    case 0xDF:
      RST(0x18);
      break;
    case 0xE0:
      LDH(readMemory(byteA));
      break;
    case 0xE1:
      POP(REG_HL);
      break;
    case 0xE2:
      LD_mem(readReg(REG_C), readReg(REG_A));
      break;
    case 0xE5:
      PUSH(REG_HL);
      break;
    case 0xE6:
      AND(byteA);
      break;
    case 0xE7:
      RST(0x20);
      break;
    case 0xE8:
      ADD_sp(byteA);
      break;
    case 0xE9:
      JP(readMemory(readReg(REG_HL)));
      break;
    case 0xEA:
      LD_mem(combineBytes(byteA, byteB), readReg(REG_A));
      break;
    case 0xEE:
      XOR(byteA);
      break;
    case 0xEF:
      RST(0x28);
      break;
    case 0xF0:
      LDH_mem(readMemory(byteA));
      break;
    case 0xF1:
      POP(REG_AF);
      break;
    case 0xF2:
      LD(REG_A, readMemory(readReg(REG_C)));
      break;
    case 0xF3:
      DI();
      break;
    case 0xF5:
      PUSH(REG_AF);
      break;
    case 0xF6:
      OR(byteA);
      break;
    case 0xF7:
      RST(0x30);
      break;
    case 0xF8:
      LD(REG_HL, sp + byteA);
      break;
    case 0xF9:
      LD_sp(readReg(REG_HL));
      break;
    case 0xFA:
      LD(REG_A, readMemory(combineBytes(byteA, byteB)));
      break;
    case 0xFB:
      EI();
      break;
    case 0xFE:
      CP(byteA);
      break;
    case 0xFF:
      RST(0x38);
      break;
    default:
      printf("Error, unimplemented opcode %#04X\n", opcode);
      exit(1);
    }
  }
  else { // extended opcode
    printf("extended opcode: 0x%02X 0x%02X 0x%02X\n", opcode, byteA, byteB);

    switch(opcode) {
    case 0x11:
      RL(REG_C);
      break;
    case 0x16:
      RL_mem(readReg(REG_HL));
      break;
    case 0x7C:
      BIT(7, readReg(REG_H));
      readNextByte(); // do length properly
      break;
    default:
      printf("Error, unimplemented extended opcode 0x%02x\n", opcode);
      exit(1);      
    }

    prefixCB = false;
  }
}

//
// 8-Bit Loads
//

void LD(registerName reg, uint8_t value) {
  writeReg(reg, value);
}

void LD_mem(uint16_t address, uint8_t value) {
  writeMemory(address, value);
}

void LDH(uint8_t offset) {
  writeMemory(0xFF00 + offset, readReg(REG_A));  
}

// 0xFF00 + offset is bigger than REG_A?
void LDH_mem(uint8_t offset) {
  writeReg(REG_A, 0xFF00 + offset);
}

//
// 16-Bit Loads
//

void LD_word(registerName reg, uint16_t value) {
  writeReg(reg, value);
}

// not sure about this function
void LD_mem_word(uint16_t address, uint16_t value) {
  writeMemory(address, getHighByte(value));
  writeMemory(address + 1, getLowByte(value));
}

void LD_sp(uint16_t value) {
  sp = value;
}

void PUSH(registerName reg) {
  pushWord(readReg(reg));  
}

void POP(registerName reg) {
  writeReg(reg, popWord());
}

//
// 8-Bit ALU
//

void ADD(uint8_t value) {
  uint8_t registerValue = readReg(REG_A);
  uint8_t result = registerValue + value;  
  setFlag('Z', result == 0);
  setFlag('N', false);
  setFlag('H', getBit(3, registerValue) && getBit(3, value));
  setFlag('C', getBit(7, registerValue) && getBit(7, value));

  writeReg(REG_A, result);
}

void ADC(uint8_t value) {
  uint8_t registerValue = readReg(REG_A);
  uint8_t result = registerValue + value + getFlag('C');
  
  setFlag('Z', result == 0);
  setFlag('N', false);
  setFlag('H', getBit(3, registerValue) && getBit(3, value));
  setFlag('C', getBit(7, registerValue) && getBit(7, value));

  writeReg(REG_A, result);
}

void SUB(uint8_t value) {
  uint8_t registerValue = readReg(REG_A);
  uint8_t result = registerValue - value;
  
  setFlag('Z', result == 0);
  setFlag('N', true);
  setFlag('H', getBit(3, registerValue) || !getBit(3, value));
  setFlag('C', getBit(7, registerValue) || !getBit(7, value));
  
  writeReg(REG_A, result);
}

void SBC(uint8_t value) {
  uint8_t registerValue = readReg(REG_A);
  uint8_t result = registerValue - value - getFlag('C');
  
  setFlag('Z', result == 0);
  setFlag('N', true);
  // unsure about H and C flags
  setFlag('H', getBit(3, registerValue) || !getBit(3, value));
  setFlag('C', getBit(7, registerValue) || !getBit(7, value));
  
  writeReg(REG_A, result);
}

void AND(uint8_t value) {
  uint8_t result = readReg(REG_A) & value;
  writeReg(REG_A, result);
  if (result == 0) {
    setFlag('Z', true);
  }
  setFlag('N', false);
  setFlag('H', true);
  setFlag('C', false);
}

void XOR(uint8_t value) {
  uint8_t result = readReg(REG_A) ^ value;
  writeReg(REG_A, result);
  if (result == 0) {
    setFlag('Z', true);
  }
  setFlag('N', false);
  setFlag('H', false);
  setFlag('C', false);
}

void OR(uint8_t value) {
  uint8_t result = readReg(REG_A) | value;
  writeReg(REG_A, result);
  if (result == 0) {
    setFlag('Z', true);
  }
  setFlag('N', false);
  setFlag('H', false);
  setFlag('C', false);
}

void CP(uint8_t value) {
  uint8_t registerValue = readReg(REG_A);
  setFlag('Z', registerValue == value);
  setFlag('N', true);
  setFlag('H', getBit(3, registerValue) || !getBit(3, value));
  setFlag('C', registerValue < value);
}


void INC(registerName reg) {
  uint8_t registerValue = readReg(reg);
  uint8_t result = registerValue + 1;

  setFlag('Z', result == 0);
  setFlag('N', false);
  setFlag('H', (registerValue & 0xF) == 0xF); // probably
    
  writeReg(reg, result);
}

void INC_mem(uint16_t address) {
  uint8_t memoryValue = readMemory(address);
  uint8_t result = memoryValue + 1;

  setFlag('Z', result == 0);
  setFlag('N', false);
  setFlag('H', (memoryValue & 0xF) == 0xF);
    
  writeMemory(address, result);
}

void DEC(registerName reg) {
  uint8_t registerValue = readReg(reg);
  uint8_t result = registerValue - 1;

  setFlag('Z', result == 0);
  setFlag('N', true);
  setFlag('H', (registerValue & 0xF) == 0);

  writeReg(reg, result);
}

void DEC_mem(uint16_t address) {
  uint8_t memoryValue = readMemory(address);
  uint8_t result = memoryValue - 1;

  setFlag('Z', result == 0);
  setFlag('N', true);
  setFlag('H', (memoryValue & 0xF) == 0);

  writeMemory(address, result);
}

//
// 16-Bit ALU
//

void ADD_word(uint16_t value) {
  uint16_t registerValue = readReg(REG_HL);
  uint16_t result = registerValue + value;
  
  setFlag('Z', result == 0);
  setFlag('N', false);
  setFlag('H', getBit(11, registerValue) && getBit(11, registerValue));
  setFlag('C', getBit(15, registerValue) && getBit(15, registerValue));

  writeReg(REG_HL, result);
}

void ADD_sp(uint8_t value) {
  sp += value;
}

void INC_sp() {
  sp++;
}

void DEC_sp() {
  sp--;
}

//
// Miscellaneous
//

void SWAP(registerName reg) {
  uint8_t value = readReg(reg);
  uint8_t result = (value << 2) | (value >> 2);
  writeReg(reg, result);
  setFlag('Z', result == 0);
  setFlag('N', 0);
  setFlag('H', 0);
  setFlag('C', 0);  
}

void SWAP_mem(uint16_t address) {
  uint8_t value = readMemory(address);
  uint8_t result = (value << 2) | (value >> 2);
  writeMemory(address, result);
  setFlag('Z', result == 0);
  setFlag('N', 0);
  setFlag('H', 0);
  setFlag('C', 0);
}

void DAA() {
  // decimal adjust register A
}

void CPL() {
  writeReg(REG_A, ~readReg(REG_B));
  setFlag('N', true);
  setFlag('H', true);
}

void CCF() {
  setFlag('C', !getFlag('C'));
  setFlag('N', false);
  setFlag('H', false);
}

void SCF() {
  setFlag('C', true);
  setFlag('N', false);
  setFlag('H', false);
}

void NOP() {
  return;
}

void HALT() {
  // power down cpu until interrupt occurs
}

void STOP() {
  // halt cpu and video until button pressed
}

void DI() {
  interruptsEnabled = false;
}

void EI() {
  interruptsEnabled = true;
}

//
// Rotates and Shifts
//

void RLCA() {
  uint8_t value = readReg(REG_A);
  bool carry = getBit(value, 7);

  value = (value << 1) | carry;
  
  if (value == 0) {
    setFlag('Z', true);
  }
  setFlag('N', false);
  setFlag('H', false);
  setFlag('C', carry);

  writeReg(REG_A, value);
}

void RLA() {
  uint8_t value = readReg(REG_A);
  bool carry = getBit(value, 7);

  value = (value << 1) | getFlag('C');

  if (value == 0) {
    setFlag('Z', true);
  }
  setFlag('N', false);
  setFlag('H', false);
  setFlag('C', carry);

  writeReg(REG_A, value);
}

void RRCA() {
  uint8_t value = readReg(REG_A);
  bool carry = getBit(value, 0);

  value = (value >> 1) | (carry << 7);

  if (value == 0) {
    setFlag('Z', true);
  }
  setFlag('N', false);
  setFlag('H', false);
  setFlag('C', carry);

  writeReg(REG_A, value);
}

void RRA() {
  uint8_t value = readReg(REG_A);
  bool carry = getBit(value, 0);

  value = (value >> 1) | (getFlag('C') << 7);

  if (value == 0) {
    setFlag('Z', true);
  }
  setFlag('N', false);
  setFlag('H', false);
  setFlag('C', carry);

  writeReg(REG_A, value);
}

//
// Bit Opcodes
//

void BIT(uint8_t bit, uint8_t value) {
  setFlag('Z', getBit(bit, value));
  setFlag('N', false);
  setFlag('H', true);
}

//
// Jumps
//

void JP(uint16_t address) {
  pc = address;
}

void JP_NZ(uint16_t address) {
  if (!getFlag('Z')) {
    pc = address;
  }
}

void JP_Z(uint16_t address) {
  if (getFlag('Z')) {
    pc = address;
  }
}

void JP_NC(uint16_t address) {
  if (!getFlag('C')) {
    pc = address;
  }
}

void JP_C(uint16_t address) {
  if (getFlag('C')) {
    pc = address;
  }
}

void JR(uint8_t offset) {
  pc += offset;
}

void JR_NZ(uint8_t offset) {
  if (!getFlag('Z')) {
    pc += offset;
  }
}

void JR_Z(uint8_t offset) {
  if (getFlag('Z')) {
    pc += offset;
  }
}

void JR_NC(uint8_t offset) {
  if (!getFlag('C')) {
    pc += offset;
  }
}

void JR_C(uint8_t offset) {
  if (getFlag('C')) {
    pc += offset;
  }
}

void CALL(uint16_t address) {
  pushWord(pc + 1);
  // swap byte order because input is LSB first
  pc = combineBytes(getLowByte(address), getHighByte(address));
}

void CALL_NZ(uint16_t address) {
  if (!getFlag('Z')) {
    CALL(address);
  }
}

void CALL_Z(uint16_t address) {
  if (getFlag('Z')) {
    CALL(address);
  }
}

void CALL_NC(uint16_t address) {
  if (!getFlag('C')) {
    CALL(address);
  }
}

void CALL_C(uint16_t address) {
  if (getFlag('C')) {
    CALL(address);
  }
}

void RST(uint8_t offset) {
  // should maybe push pc+1
  pushWord(pc);
  pc = offset;
}

void RET() {
  pc = popWord();
}

void RET_NZ() {
  if (!getFlag('Z')) {
    pc = popWord();
  }
}

void RET_Z() {
  if (getFlag('Z')) {
    pc = popWord();
  }
}

void RET_NC() {
  if (!getFlag('C')) {
    pc = popWord();
  }
}

void RET_C() {
  if (getFlag('C')) {
    pc = popWord();
  }
}

void RETI() {
  RET();
  interruptsEnabled = true;
}
