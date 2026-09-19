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

void ram_init();

uint8_t read_ram8( uint32_t addr );

void write_ram8( uint32_t addr, uint8_t val );

// reads from 0xffff will have incorrect high byte for 8086
uint16_t read_ram16( uint32_t addr );

// write to 0xffff will place high byte in wrong location for 8086
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
