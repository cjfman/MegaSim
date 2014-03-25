// core.c
//
// Developed by Charles Franklin
//
// MIT Lisense
//

#include "core.h"

// Initialize pointers
uint16_t *regps = (uint16_t*)regs; // Register pairs
uint16_t *RW = (uint16_t*)(&regs[24]);  // R25:R24
uint16_t *RX = (uint16_t*)(&regs[26]);  // R27:R26
uint16_t *RY = (uint16_t*)(&regs[28]);  // R29:R28
uint16_t *RZ = (uint16_t*)(&regs[30]);  // R31:R30
