#include "georam.h"

/*
0xdfff selects 16 kB block within 4 MB
0xdffe selects 256 byte page within 16 kB block
page frame is 0xde00 to 0xdff
*/

volatile uint8_t* CIA1_TOD = (void*)0xdc08;
volatile uint8_t* CIA1_TOD_0 = (void*)0xdc08;
volatile uint8_t* CIA1_TOD_1 = (void*)0xdc09;
volatile uint8_t* CIA1_TOD_2 = (void*)0xdc0a;
volatile uint8_t* CIA1_TOD_3 = (void*)0xdc0b;
volatile uint8_t* CIA2_TOD = (void*)0xdd08;

uint8_t read_ram8( uint32_t addr ) {
    *(volatile uint8_t*)0xdfff = addr >> 14;
    *(volatile uint8_t*)0xdffe = addr >> 8 & 0x3f;
    return ((volatile uint8_t*)0xde00)[addr & 0xff];
}
void write_ram8( uint32_t addr, volatile uint8_t data ) {
    *(volatile uint8_t*)0xdfff = addr >> 14;
    *(volatile uint8_t*)0xdffe = addr >> 8 & 0x3f;
    ((volatile uint8_t*)0xde00)[addr & 0xff] = data;
}

// reads from 0xffff will have incorrect high byte
uint16_t read_ram16( uint32_t addr ) {
  *(volatile uint8_t*)0xdfff = addr >> 14;
  *(volatile uint8_t*)0xdffe = addr >> 8 & 0x3f;
  if( (addr & 255) < 255 ) {
    return *(uint16_t*)&((volatile uint8_t*)0xde00)[addr & 0xff];
  } else {
    uint16_t temp = ((volatile uint8_t*)0xde00)[255];
    *(volatile uint8_t*)0xdfff = (addr + 256) >> 14;
    *(volatile uint8_t*)0xdffe = (addr + 256) >> 8 & 0x3f;
    return temp | (uint16_t)((volatile uint8_t*)0xde00)[0] << 8;
  }
}

// write to 0xffff will have incorrect high byte,
// but that appears to be consistent with tht the 80186 does
void write_ram16( uint32_t addr, uint16_t val ) {
  *(volatile uint8_t*)0xdfff = addr >> 14;
  *(volatile uint8_t*)0xdffe = addr >> 8 & 0x3f;
  if( (addr & 255) < 255 ) {
    *(uint16_t*)&((volatile uint8_t*)0xde00)[addr & 0xff] = val;
  } else {
    ((volatile uint8_t*)0xde00)[255] = val;
    *(volatile uint8_t*)0xdfff = (addr + 256) >> 14;
    *(volatile uint8_t*)0xdffe = (addr + 256) >> 8 & 0x3f;
    ((volatile uint8_t*)0xde00)[0] = val >> 8;
  }
}

uint8_t read_io_ports8( uint16_t addr ) {
  return read_ram8( IO_START + addr );
}

void write_io_ports8( uint16_t addr, uint8_t val ) {
  write_ram8( IO_START + addr, val );
}

uint16_t read_io_ports16( uint16_t addr ) {
  return read_ram16( IO_START + addr );
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

void getrtc(){
  volatile uint8_t temp;
  temp = *CIA1_TOD_3;
  temp = (temp & 0xf) + (temp & 0x10)/8*5 + (temp & 0x80)/32*3;
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 8, temp );
  temp = *CIA1_TOD_2;
  temp = (temp & 0xf) + (temp & 0xf0)/8*5;
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 4, temp );
  temp = *CIA1_TOD_1;
  temp = (temp & 0xf) + (temp & 0xf0)/8*5;
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX), temp );
  write_ram16(16L * read_regs16(REG_ES) + (unsigned short)( 36+ read_regs16( REG_BX)), *CIA1_TOD_0 * 100 );}

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
  *CIA1_TOD_0 = 0;
  volatile uint8_t temp;
  temp = *CIA1_TOD_3;
  temp = (temp & 0xf) + (temp & 0x10)/8*5 + (temp & 0x80)/32*3;
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 8, temp );
  temp = *CIA1_TOD_2;
  temp = (temp & 0xf) + (temp & 0xf0)/8*5;
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 4, temp );
  temp = *CIA1_TOD_1;
  temp = (temp & 0xf) + (temp & 0xf0)/8*5;
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX), temp );
  write_ram16(16L * read_regs16(REG_ES) + (unsigned short)( 36+ read_regs16( REG_BX)), *CIA1_TOD_0 * 100 );
}

uint8_t read_disk( int whichdisk, uint32_t addr ) {
  return read_ram8( DISKA_START + addr );
}

int write_disk( int whichdisk, uint32_t addr, uint8_t val ) {
  write_ram8( DISKA_START + addr, val );
  return 0;
}
