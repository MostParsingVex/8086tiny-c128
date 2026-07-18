#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h>
#include"8086tiny.h"

#define RAM_SIZE 0x10FFF0L
#define IO_PORT_COUNT 0x10000L
#define REGS_BASE 0xF0000L

time_t clock_buf;
struct timeb ms_clock;
//extern FILE*logfd;

#if 0
uint8_t mem[ RAM_SIZE ];
uint8_t io_ports[ IO_PORT_COUNT ];
#else
uint8_t *mem, *io_ports;
#endif

uint8_t read_ram8( uint32_t addr ) {
  return mem[ addr ];
}

void write_ram8( uint32_t addr, uint8_t val ) {
//if(addr<0xf0000L)fprintf(logfd, " w %05lx %02x\n",addr, val);
  mem[ addr ] = val;
}

// reads from 0xffff will have incorrect high byte
uint16_t read_ram16( uint32_t addr ) {
  //return *(uint16_t*)(mem + addr);
  return read_ram8( addr ) | (uint16_t)read_ram8( addr + 1 ) << 8;
}

// write to 0xffff will have incorrect high byte,
// but that appears to be consistent with tht the 80186 does
void write_ram16( uint32_t addr, uint16_t val ) {
  //*(uint16_t*)(mem + addr) = val;
  write_ram8( addr, val );
  write_ram8( addr + 1, val >> 8 );
}

uint8_t read_io_ports8( uint16_t addr ) {
//printf("read_io_ports addr %04x\n", addr );
  return io_ports[ addr ];
}

void write_io_ports8( uint16_t addr, uint8_t val ) {
//printf("write_io_ports8 addr %04x val %d\n", addr, val);
  io_ports[ addr ] = val;
}

uint16_t read_io_ports16( uint16_t addr ) {
//printf("read_io_ports16 addr %04x\n", addr);
//return *(uint16_t*)&io_ports[ addr ];
//return io_ports[ addr ] | (uint16_t)io_ports[ addr + 1 ] << 8; 
  return read_io_ports8( addr ) | (uint16_t)read_io_ports8( addr + 1 ) << 8;
}

void write_io_ports16( uint16_t addr, uint16_t val ) {
//printf("write_io_ports16 addr %04x val %d\n", addr, val);
//*(uint16_t*)&io_ports[ addr ] = val;
  write_io_ports8( addr, val);
  write_io_ports8( addr + 1, val >> 8);
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
//bios_table_lookup[i][j] = regs8[regs16[0x81 + i] + j];
  return read_regs8( read_regs16( 0x81 + i ) + j );
}
#if 0
uint8_t diska[ 1 << 21 ];
uint8_t diskc[ 1 << 24 ];
#else
uint8_t *diska, *diskc;
#endif

uint8_t read_disk( int whichdisk, uint32_t addr ) {
//printf("whichdisk %d addr %08x\n",whichdisk, addr);
  return *((whichdisk&0 ? diskc : diska) + addr);
}

int write_disk( int whichdisk, uint32_t addr, uint8_t val ) {
  *((whichdisk&0 ? diskc : diska) + addr) = val;
  return 0;
}

void getrtc(){
#if 1
  time(&clock_buf);
  ftime(&ms_clock);
  {
    char* tmptr = (char*)localtime(&clock_buf);
    for( int i = 0; i < sizeof(struct tm); i++ ) {
      write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + i, tmptr[ i ] );
    }
  }
  write_ram16(16 * read_regs16(REG_ES) + (unsigned short)( 36+ read_regs16( REG_BX)), ms_clock.millitm );
#if 0
for(int i = 0;i<40;i++)
printf("%02x ", read_ram8( 16 * read_regs16(REG_ES) + (unsigned short)( read_regs16( REG_BX)) + i));
puts("");
exit(0);
#endif
#endif
}

int read_console( uint8_t *buffer ) {
  return read( 0, buffer, 1 );
}

int write_console( uint8_t *buffer ) {
  return write( 1, buffer, 1 );
}

void ram_init() {
  mem = calloc( RAM_SIZE,1 );
  io_ports = calloc( IO_PORT_COUNT,1 );
  diska = malloc( 1 << 21 );
  diskc = malloc( 1 << 24 );

  int fd = open("bios",32898);
  for( uint32_t i = 0; i < 0xff00; i++ ) {
    uint8_t temp;
    read( fd, &temp, 1 );
    write_ram8( 0xf0100 + i, temp );
  }
  close( fd );

  fd = open("fd.img",32898);
  for( int32_t i = 0; i < 1 << 21; i++ ) {
    uint8_t temp;
    read( fd, &temp, 1 );
    write_disk( 0, i, temp );
  }
  close(fd);
}
