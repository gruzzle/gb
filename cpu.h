#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "memory.h"

typedef enum { REG_A, REG_B, REG_C, REG_D, REG_E, REG_F,
	       REG_H, REG_L, REG_AF, REG_BC, REG_DE, REG_HL } registerName;

void initializeCPU(void);
void mainLoop(void);
uint8_t readNextByte(void);
void executeOpcode(uint8_t);

uint8_t getHighByte(uint16_t);
uint8_t getLowByte(uint16_t);
uint16_t combineBytes(uint8_t, uint8_t);
uint8_t getBit(uint8_t, uint8_t);
void print16(uint16_t);
void printRegisters(void);
const char *byteToBinary(uint8_t);

void push(uint8_t);
void pushWord(uint16_t);
uint8_t pop(void);
uint16_t popWord(void);
void writeReg(registerName, uint16_t);
uint16_t readReg(registerName);
uint8_t getFlag(char);
void setFlag(char, bool);

// 8-Bit Loads
void LD(registerName, uint8_t);
void LD_mem(uint16_t, uint8_t);
void LDH(uint8_t);
void LDH_mem(uint8_t);

// 16-Bit Loads
void LD_word(registerName, uint16_t);
void LD_mem_word(uint16_t, uint16_t);
void LD_sp(uint16_t);
void PUSH(registerName);
void POP(registerName);

// 8-Bit ALU
void ADD(uint8_t);
void ADC(uint8_t);
void SUB(uint8_t);
void SBC(uint8_t);
void AND(uint8_t);
void XOR(uint8_t);
void OR(uint8_t);
void CP(uint8_t);
void INC(registerName);
void INC_mem(uint16_t);
void DEC(registerName);
void DEC_mem(uint16_t);

// 16-Bit ALU
void ADD_word(uint16_t);
void ADD_sp(uint8_t);
void INC_sp(void);
void DEC_sp(void);

// Miscellaneous
void SWAP(registerName);
void SWAP_mem(uint16_t);
void DAA(void);
void CPL(void);
void CCF(void);
void SCF(void);
void NOP(void);
void HALT(void);
void STOP(void);
void DI(void);
void EI(void);

// Rotates and Shifts
void RLCA(void);
void RLA(void);
void RRCA(void);
void RRA(void);
void RLC(registerName); //
void RLC_mem(registerName); //
void RL(registerName); //
void RL_mem(uint16_t); //
void RRC(registerName); //
void RRC_mem(uint16_t); //
void SLA(registerName); //
void SLA_mem(uint16_t); //
void SRA(registerName); //
void SRA_mem(uint16_t); //
void SRL(registerName); //
void SRL_mem(uint16_t); //

// Bit Opcodes
void BIT(uint8_t, uint8_t);
void SET(registerName, uint8_t); //
void RES(registerName, uint8_t); //
void RES_mem(uint16_t, uint8_t); //

// Jumps
void JP(uint16_t);
void JP_NZ(uint16_t);
void JP_Z(uint16_t);
void JP_NC(uint16_t);
void JP_C(uint16_t);
void JR(uint8_t);
void JR_NZ(uint8_t);
void JR_Z(uint8_t);
void JR_NC(uint8_t);
void JR_C(uint8_t);
void CALL(uint16_t);
void CALL_NZ(uint16_t);
void CALL_Z(uint16_t);
void CALL_NC(uint16_t);
void CALL_C(uint16_t);
void RST(uint8_t);
void RET(void);
void RET_NZ(void);
void RET_Z(void);
void RET_NC(void);
void RET_C(void);
void RETI(void);

#endif
