#include<stdint.h>
#include<ctype.h>

uint8_t read_ram8( uint32_t addr );

void write_ram8( uint32_t addr, uint8_t val );

// reads from 0xffff will have incorrect high byte
uint16_t read_ram16( uint32_t addr );

// write to 0xffff will have incorrect high byte,
// but that appears to be consistent with tht the 80186 does
void write_ram16( uint32_t addr, uint16_t val );

uint8_t read_io_ports8( uint16_t addr );

void write_io_ports8( uint16_t addr, uint8_t val );

uint16_t read_io_ports16( uint16_t addr );

void write_io_ports16( uint16_t addr, uint16_t val );

uint8_t read_regs8( uint32_t addr );

void write_regs8( uint8_t addr, uint8_t val );

uint16_t read_regs16( uint32_t addr );

void write_regs16( uint32_t addr, uint16_t val );

uint8_t read_bios_table_lookup( uint8_t i, uint8_t j );

uint8_t read_disk( int whichdisk, uint32_t addr );

int write_disk( int whichdisk, uint32_t addr, uint8_t val );

void getrtc();

void ram_init();
