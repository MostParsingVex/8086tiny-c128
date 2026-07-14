#include"w65816.h"
//#include"y88.h"
#include<string.h>

uint8_t read_ram8( uint32_t addr ) {
    // lda addr + 0x020000
    2[(uint8_t*)&addr] += 2;
    memcpy( &&read_label, &addr, 3 );
    uint8_t val;
    asm volatile( ".byte 0xaf\n" );
    read_label:
    asm( ".byte 0, 0, 0\n" : "=a"(val) :: );
    return val;
}

void write_ram8( uint32_t addr, uint8_t data ) {
    // sta addr + 0x020000
    2[(uint8_t*)&addr] += 2;
    memcpy( &&write_label, &addr, 3 );
    asm volatile( ".byte 0x8f\n" :: "a"(data) : );
    write_label:
    asm volatile( ".byte 0, 0, 0\n" );
}

// reads from 0xffff will have incorrect high byte
uint16_t read_ram16( uint32_t addr ) {
  return read_ram8( addr ) | (uint16_t)read_ram8( addr + 1 ) << 8;
}

// write to 0xffff will have incorrect high byte,
// but that appears to be consistent with tht the 80186 does
void write_ram16( uint32_t addr, uint16_t val ) {
  write_ram8( addr, val );
  write_ram8( addr + 1, val >> 8 );
}

uint8_t read_io_ports8( uint16_t addr ) {
  return read_ram8( IO_START + addr );
}

void write_io_ports8( uint16_t addr, uint8_t val ) {
  write_ram8( IO_START + addr, val );
}

uint16_t read_io_ports16( uint16_t addr ) {
  return read_io_ports8(addr) | (uint16_t)read_io_ports8(addr+1) << 8;
}

void write_io_ports16( uint16_t addr, uint16_t val ) {
  write_io_ports8(addr, val);
  write_io_ports8(addr+1, val >> 8);
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
}

uint8_t read_disk( int whichdisk, uint32_t addr ) {
  return read_ram8( DISKA_START + addr );
}

int write_disk( int whichdisk, uint32_t addr, uint8_t val ) {
  write_ram8( DISKA_START + addr, val );
  return 0;
}

