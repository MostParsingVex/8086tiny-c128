#include "reu.h"

#define RAM_START 0x000000L
#define BIOS_START 0xf0000L
#define IO_START 0x110000L
#define DISKA_START 0x200000L
#define DISKC_START 0x400000L

#define REGS_BASE 0xF0000

volatile uint8_t* REU_STATUS=(void*)0xdf00;
volatile uint8_t* REU_CMD=(void*)0xdf01;
volatile uint16_t* REU_C64ADR=(void*)0xdf02;
volatile uint16_t* REU_ADDRLO=(void*)0xdf04;
volatile uint8_t* REU_ADDRHI=(void*)0xdf06;
volatile uint16_t* REU_XFERLEN=(void*)0xdf07;

volatile uint8_t* REU_CONTROL=(void*)0xdf0a;

volatile uint8_t* VIC_2MHZ=(void*)0xd030;

#include<cbm.h>
#include<stdio.h>

uint32_t loadword_reu(uint32_t addr) {
    volatile uint32_t value;
    *REU_C64ADR = (uint16_t)(&value);
    *REU_ADDRLO = addr & 0xffff;
    *REU_ADDRHI = addr >> 16;
    *REU_XFERLEN = 4;
    *REU_CMD = 0b10010001;
    return value;
}

void saveword_reu(uint32_t addr, volatile uint32_t value) {
    *REU_C64ADR = (uint16_t)(&value);
    *REU_ADDRLO = addr & 0xffff;
    *REU_ADDRHI = addr >> 16;
    *REU_XFERLEN = 4;
    *REU_CMD = 0b10010000;
}


void load_from_reu(void* dest, uint32_t reu_addr, uint16_t len) {
    *REU_C64ADR = (uint16_t)dest;
    *REU_ADDRLO = reu_addr & 0xffff;
    *REU_ADDRHI = reu_addr >> 16;
    *REU_XFERLEN = len;
    *REU_CMD = 0b10010001;
}

void save_to_reu(uint32_t reu_addr, void* src, uint16_t len) {
    *REU_C64ADR = (uint16_t)src;
    *REU_ADDRLO = reu_addr & 0xffff;
    *REU_ADDRHI = reu_addr >> 16;
    *REU_XFERLEN = len;
    *REU_CMD = 0b10010000;
}

uint16_t load16_reu(uint32_t addr) {
    volatile uint16_t value;
    *REU_C64ADR = (uint16_t)(&value);
    *REU_ADDRLO = addr & 0xffff;
    *REU_ADDRHI = addr >> 16;
    *REU_XFERLEN = 2;
    *REU_CMD = 0b10010001;
    return value;
}

void save16_reu(uint32_t addr, volatile uint16_t value) {
    *REU_C64ADR = (uint16_t)(&value);
    *REU_ADDRLO = addr & 0xffff;
    *REU_ADDRHI = addr >> 16;
    *REU_XFERLEN = 2;
    *REU_CMD = 0b10010000;
}

uint8_t read_ram8(uint32_t addr) {
  volatile uint8_t temp;
  uint16_t ptr = (uint16_t)&temp;

#ifdef C128
  *VIC_2MHZ = 252;
#endif
  *REU_C64ADR = ptr;
  *REU_CONTROL = 0;
  *REU_ADDRLO = addr;
  *REU_ADDRHI = addr >> 16;
  *REU_XFERLEN = 1;
  *REU_CMD = 0x91;
#ifdef C128
  *VIC_2MHZ = 253;
#endif
  return temp;
}
void write_ram8(uint32_t addr, volatile uint8_t value) {
  uint16_t ptr = (uint16_t)&value;
#ifdef C128
  *VIC_2MHZ = 252;
#endif
  *REU_C64ADR = ptr;
  *REU_CONTROL = 0;
  *REU_ADDRLO = addr;
  *REU_ADDRHI = addr >> 16;
  *REU_XFERLEN = 1;
  *REU_CMD = 0x90;
#ifdef C128
  *VIC_2MHZ = 253;
#endif
}

// reads from 0xffff will have incorrect high byte
uint16_t read_ram16( uint32_t addr ) {
  volatile uint16_t temp;
  uint16_t ptr = (uint16_t)&temp;

#ifdef C128
  *VIC_2MHZ = 252;
#endif
  *REU_C64ADR = ptr;
  *REU_CONTROL = 0;
  *REU_ADDRLO = addr;
  *REU_ADDRHI = addr >> 16;
  *REU_XFERLEN = 2;
  *REU_CMD = 0x91;
#ifdef C128
  *VIC_2MHZ = 253;
#endif
  return temp;
}

// write to 0xffff will have incorrect high byte,
// but that appears to be consistent with tht the 80186 does
void write_ram16( uint32_t addr, uint16_t val ) {
  uint16_t ptr = (uint16_t)&val;
#ifdef C128
  *VIC_2MHZ = 252;
#endif
  *REU_C64ADR = ptr;
  *REU_CONTROL = 0;
  *REU_ADDRLO = addr;
  *REU_ADDRHI = addr >> 16;
  *REU_XFERLEN = 2;
  *REU_CMD = 0x90;
#ifdef C128
  *VIC_2MHZ = 253;
#endif
}

uint8_t read_io_ports8( uint16_t addr ) {
  return read_ram8( IO_START + addr );
}

void write_io_ports8( uint16_t addr, uint8_t val ) {
  write_ram8( IO_START + addr, val );
}

uint16_t read_io_ports16( uint16_t addr ) {
  return read_ram16(IO_START + addr);
}

void write_io_ports16( uint16_t addr, uint16_t val ) {
  write_ram16( IO_START + addr, val );
}

uint8_t read_regs8( uint32_t addr ) {
  return read_ram8( REGS_BASE + addr );
}

void write_regs8( uint8_t addr, uint8_t val ) {
  write_ram8( REGS_BASE + addr, val );
}

uint16_t read_regs16( uint32_t addr ) {
  return read_ram16( REGS_BASE + addr * 2 );
}

void write_regs16( uint32_t addr, uint16_t val ) {
  write_ram16( REGS_BASE + addr * 2, val );
}

uint8_t read_bios_table_lookup( uint8_t i, uint8_t j ) {
  return read_regs8( read_regs16( 0x81 + i ) + j );
}

void getrtc(){}

void cbm_k_chrout_wrapper( uint8_t input ) {
  if( isalpha( input ) ) {
    input ^= 32;
  }
  cbm_k_chrout( input );
}

int read_console( uint8_t *buffer ) {
  uint8_t temp = cbm_k_getin();
  if( temp ) {
    *buffer = temp;
    return 1;
  } else {
    return 0;
  }
}

int write_console( uint8_t *buffer ) {
#ifdef C128
  if(*buffer-'\n')
#endif
    cbm_k_chrout_wrapper( *buffer );
  return 1;
}

void ram_init(){
#ifdef C128
  *VIC_2MHZ = 1;
#endif
}

uint8_t read_disk( int whichdisk, uint32_t addr ) {
  return read_ram8( DISKA_START + addr );
}

int write_disk( int whichdisk, uint32_t addr, uint8_t val ) {
  write_ram8( DISKA_START + addr, val );
  return 0; 
}
