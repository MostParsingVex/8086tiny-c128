#pragma once
#include <stdint.h>
#include<ctype.h>
#include<cbm.h>

#define RAM_START 0x000000L
#define BIOS_START 0xf0000L
#define IO_START 0x110000L
#define DISKA_START 0x200000L
#define DISKC_START 0x400000L

#define REGS_BASE 0xF0000

// 16-bit register decodes
#define REG_AX 0
#define REG_CX 1
#define REG_DX 2
#define REG_BX 3
#define REG_SP 4
#define REG_BP 5
#define REG_SI 6
#define REG_DI 7

#define REG_ES 8
#define REG_CS 9
#define REG_SS 10
#define REG_DS 11

#define REG_ZERO 12
#define REG_SCRATCH 13
