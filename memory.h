#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include <stdint.h>
#include <stdio.h>
#include <string.h>

void initializeMemory(void);
uint8_t readMemory(uint16_t);
void writeMemory(uint16_t, uint8_t);

#endif
