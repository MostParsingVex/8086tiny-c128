#include"w65816.h"
#include<stdio.h>
#include<string.h>

volatile uint8_t* CIA1_TOD = (void*)0xdc08;
volatile uint8_t* CIA1_TOD_0 = (void*)0xdc08;
volatile uint8_t* CIA1_TOD_1 = (void*)0xdc09;
volatile uint8_t* CIA1_TOD_2 = (void*)0xdc0a;
volatile uint8_t* CIA1_TOD_3 = (void*)0xdc0b;
volatile uint8_t* CIA2_TOD = (void*)0xdd08;

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

/* FIXME: delete this temporary hack after we figure out how to keep time */
uint8_t tod_hack[4];

void getrtc(){
#if 0
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
#else
  write_ram16(16L * read_regs16(REG_ES) + (unsigned short)( 36+ read_regs16( REG_BX)), (uint16_t)tod_hack[0] * 100 );
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX), tod_hack[1]);
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 4, tod_hack[2]);
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 8, tod_hack[3]);
  tod_hack[0]++;
  if( tod_hack[0] >= 10 || 1 ) {
    tod_hack[0] = 0;
    tod_hack[1]++;
    if( tod_hack[1] >= 60 ) {
      tod_hack[1] = 0;
      tod_hack[2]++;
      if( tod_hack[2] >= 60 ) {
        tod_hack[2] = 0;
        tod_hack[3]++;
        if( tod_hack[3] >= 24 ) {
          tod_hack[3] = 0;
        }
      }
    }
  }
#endif
}

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
#if 0
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
#else
  write_ram16(16L * read_regs16(REG_ES) + (unsigned short)( 36+ read_regs16( REG_BX)), (uint16_t)tod_hack[0] * 100 );
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX), tod_hack[1]);
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 4, tod_hack[2]);
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 8, tod_hack[3]);
  tod_hack[0]++;
  if( tod_hack[0] >= 10 ) {
    tod_hack[0] = 0;
    tod_hack[1]++;
    if( tod_hack[1] >= 60 ) {
      tod_hack[1] = 0;
      tod_hack[2]++;
      if( tod_hack[2] >= 60 ) {
        tod_hack[2] = 0;
        tod_hack[3]++;
        if( tod_hack[3] >= 24 ) {
          tod_hack[3] = 0;
        }
      }
    }
  }
#endif
//printf("%02d:%02d:%02d:%04d\n",read_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 8 ), read_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 4 ), read_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) ), read_ram16( 16L * read_regs16(REG_ES) + (unsigned short)( 36+ read_regs16( REG_BX))) );

  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 12, 1);
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 16, 0);
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 20, 100);
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 24, 6);
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 28, 0);
  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + 32, 0);
for(int i = 0;i<40;i++) 
printf("%02x ", read_ram8( 16 * read_regs16(REG_ES) + (unsigned short)( read_regs16( REG_BX)) + i));
puts("");
}

uint8_t read_disk( int whichdisk, uint32_t addr ) {
  return read_ram8( DISKA_START + addr );
}

int write_disk( int whichdisk, uint32_t addr, uint8_t val ) {
  write_ram8( DISKA_START + addr, val );
  return 0;
}

