// 8086tiny: a tiny, highly functional, highly portable PC emulator/VM
// Copyright 2013-14, Adrian Cable (adrian.cable@gmail.com) - http://www.megalith.co.uk/8086tiny
//
// Revision 1.25
//
// This work is licensed under the MIT License. See included LICENSE.TXT.

#include<stdio.h>
#include<stdint.h>
//#include <memory.h>

#ifndef _WIN32
#include <unistd.h>
#include <fcntl.h>
#endif

#ifndef NO_GRAPHICS
#include "SDL.h"
#endif

// Emulator system constants
#define IO_PORT_COUNT 0x10000
#define RAM_SIZE 0x10FFF0
#define REGS_BASE 0xF0000
#define VIDEO_RAM_SIZE 0x10000

// Graphics/timer/keyboard update delays (explained later)
#ifndef GRAPHICS_UPDATE_DELAY
#define GRAPHICS_UPDATE_DELAY 360000
#endif
//#define KEYBOARD_TIMER_UPDATE_DELAY 20000
#define KEYBOARD_TIMER_UPDATE_DELAY 1000

#include"8086tiny.h"
FILE *logfd;

void getrtc();
int read_console( uint8_t *buffer );
int write_console( uint8_t *buffer );

// 8-bit register decodes
#define REG_AL 0
#define REG_AH 1
#define REG_CL 2
#define REG_CH 3
#define REG_DL 4
#define REG_DH 5
#define REG_BL 6
#define REG_BH 7

// FLAGS register decodes
#define FLAG_CF 40
#define FLAG_PF 41
#define FLAG_AF 42
#define FLAG_ZF 43
#define FLAG_SF 44
#define FLAG_TF 45
#define FLAG_IF 46
#define FLAG_DF 47
#define FLAG_OF 48

// Lookup tables in the BIOS binary
#define TABLE_XLAT_OPCODE 8
#define TABLE_XLAT_SUBFUNCTION 9
#define TABLE_STD_FLAGS 10
#define TABLE_PARITY_FLAG 11
#define TABLE_BASE_INST_SIZE 12
#define TABLE_I_W_SIZE 13
#define TABLE_I_MOD_SIZE 14
#define TABLE_COND_JUMP_DECODE_A 15
#define TABLE_COND_JUMP_DECODE_B 16
#define TABLE_COND_JUMP_DECODE_C 17
#define TABLE_COND_JUMP_DECODE_D 18
#define TABLE_FLAGS_BITFIELDS 19

// Bitfields for TABLE_STD_FLAGS values
#define FLAGS_UPDATE_SZP 1
#define FLAGS_UPDATE_AO_ARITH 2
#define FLAGS_UPDATE_OC_LOGIC 4

# 90 "8086tiny.c"

// Helper macros

// Decode mod, r_m and reg fields in instruction





// Return memory-mapped register location (offset into mem array) for register #reg_id


// Returns number of top bit in operand (i.e. 8 for 8-bit operands, 16 for 16-bit operands)


// Opcode execution unit helpers



// [I]MUL/[I]DIV/DAA/DAS/ADC/SBB helpers

# 120 "8086tiny.c"

// Execute arithmetic/logic operations in emulator memory/registers





// Increment or decrement a register #reg_id (usually SI or DI), depending on direction flag and operand size (given by i_w)


// Helpers for stack operations



// Convert segment:offset to linear address in emulator memory space


// Returns sign bit of an 8-bit or 16-bit operand


// Reinterpretation cast


// Keyboard driver for console. This may need changing for UNIX/non-UNIX platforms






// Keyboard driver for SDL






// Global variable definitions
//uint8_t mem[RAM_SIZE], io_ports[IO_PORT_COUNT], *opcode_stream,/* *regs8,*/ i_rm, i_w, i_reg, i_mod, i_mod_size, i_d, i_reg4bit, raw_opcode_id, xlat_opcode_id, extra, rep_mode, seg_override_en, rep_override_en, trap_flag, int8_asap, scratch_uchar, io_hi_lo, *vid_mem_base, spkr_en, bios_table_lookup[20][256];
uint8_t i_rm, i_w, i_reg, i_mod, i_mod_size, i_d, i_reg4bit, raw_opcode_id, xlat_opcode_id, extra, rep_mode, seg_override_en, rep_override_en, trap_flag, int8_asap, scratch_uchar, io_hi_lo, *vid_mem_base, spkr_en;
//uint16_t /* *regs16,*/ reg_ip, seg_override, file_index, wave_counter;
uint16_t reg_ip, seg_override, file_index, wave_counter;
uint32_t opcode_stream, op_source, op_dest, rm_addr, op_to_addr, op_from_addr, i_data0, i_data1, i_data2, scratch_uint, scratch2_uint, inst_counter, set_flags_type, GRAPHICS_X, GRAPHICS_Y, pixel_colors[16], vmem_ctr;
int32_t op_result, disk[3], scratch_int;
//time_t clock_buf;
//struct timeb ms_clock;

void ram_init();
uint8_t read_ram8( uint32_t addr );

void write_ram8( uint32_t addr, uint8_t val );

// reads from 0xffff will have incorrect high byte
uint16_t read_ram16( uint32_t addr );

// write to 0xffff will have incorrect high byte,
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

#if 0
SDL_AudioSpec sdl_audio = {44100, AUDIO_U8, 1, 0, 128};
SDL_Surface *sdl_screen;
SDL_Event sdl_event;
unsigned short vid_addr_lookup[VIDEO_RAM_SIZE], cga_colors[4] = {0 , 0x1F1F , 0xE3E3 , 0xFFFF };
#endif

// Helper functions

// Set carry flag
char set_CF(int new_CF)
{
	//return regs8[FLAG_CF] = !!new_CF;
        write_regs8( FLAG_CF, !!new_CF );
        return !!new_CF;
}

// Set auxiliary flag
char set_AF(int new_AF)
{
	//return regs8[FLAG_AF] = !!new_AF;
        write_regs8( FLAG_AF, !!new_AF );
        return !!new_AF;
}

// Set overflow flag
char set_OF(int new_OF)
{
	//return regs8[FLAG_OF] = !!new_OF;
        write_regs8( FLAG_OF, !!new_OF );
        return !!new_OF;
}

// Set auxiliary and overflow flag after arithmetic operations
char set_AF_OF_arith()
{
	set_AF((op_source ^= op_dest ^ op_result) & 0x10);
	if (op_result == op_dest)
		return set_OF(0);
	else
		//return set_OF(1 & (regs8[FLAG_CF] ^ op_source >> (8*(i_w + 1) - 1)));
                return set_OF(1 & (read_regs8(FLAG_CF) ^ op_source >> (8*(i_w + 1) - 1)));
}

// Assemble and return emulated CPU FLAGS register in scratch_uint
void make_flags()
{
	scratch_uint = 0xF002; // 8086 has reserved and unused flags set to 1
	for (int i = 9; i--;)
		//scratch_uint += regs8[FLAG_CF + i] << bios_table_lookup[TABLE_FLAGS_BITFIELDS][i];
                scratch_uint += read_regs8(FLAG_CF + i) << read_bios_table_lookup(TABLE_FLAGS_BITFIELDS, i);
}

// Set emulated CPU FLAGS register from regs8[FLAG_xx] values
void set_flags(int new_flags)
{
	for (int i = 9; i--;)
		//regs8[FLAG_CF + i] = !!(1 << bios_table_lookup[TABLE_FLAGS_BITFIELDS][i] & new_flags);
                write_regs8(FLAG_CF + i, !!(1 << read_bios_table_lookup(TABLE_FLAGS_BITFIELDS, i) & new_flags));
}

// Convert raw opcode to translated opcode index. This condenses a large number of different encodings of similar
// instructions into a much smaller number of distinct functions, which we then execute
void set_opcode(unsigned char opcode)
{
#if 0
	xlat_opcode_id = bios_table_lookup[TABLE_XLAT_OPCODE][raw_opcode_id = opcode];
	extra = bios_table_lookup[TABLE_XLAT_SUBFUNCTION][opcode];
	i_mod_size = bios_table_lookup[TABLE_I_MOD_SIZE][opcode];
	set_flags_type = bios_table_lookup[TABLE_STD_FLAGS][opcode];
#else
        xlat_opcode_id = read_bios_table_lookup(TABLE_XLAT_OPCODE, raw_opcode_id = opcode);
        extra = read_bios_table_lookup(TABLE_XLAT_SUBFUNCTION, opcode);
        i_mod_size = read_bios_table_lookup(TABLE_I_MOD_SIZE, opcode);
        set_flags_type = read_bios_table_lookup(TABLE_STD_FLAGS, opcode);
#endif
}

// Execute INT #interrupt_num on the emulated machine
char pc_interrupt(unsigned char interrupt_num)
{
        uint16_t sp;
	set_opcode(0xCD); // Decode like INT

	make_flags();
	//(i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&scratch_uint) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&scratch_uint))));
        (i_w = 1, (sp = read_regs16(REG_SP)-1, op_dest = read_ram16(16L * read_regs16(REG_SS) + sp), sp--, write_ram16(16L * read_regs16(REG_SS) + sp, op_result = op_source = *(unsigned short*)&scratch_uint), write_regs16( REG_SP, sp ) ));
	//(i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&regs16[REG_CS]) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&regs16[REG_CS]))));
        sp = read_regs16(REG_SP)-1, op_dest = read_ram16(16L * read_regs16(REG_SS) + sp), sp--, write_ram16(16L * read_regs16(REG_SS) + sp, op_result = op_source = read_regs16(REG_CS)), write_regs16( REG_SP, sp );
	//(i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&reg_ip) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&reg_ip))));
        sp = read_regs16(REG_SP)-1, op_dest = read_ram16(16L * read_regs16(REG_SS) + sp), sp--, write_ram16(16L * read_regs16(REG_SS) + sp, op_result = op_source = *(unsigned short*)&reg_ip), write_regs16( REG_SP, sp );
	//(i_w ? op_dest = *(unsigned short*)&mem[REGS_BASE + 2 * REG_CS], op_result = *(unsigned short*)&mem[REGS_BASE + 2 * REG_CS]  = (op_source = *(unsigned short*)&mem[ 4 * interrupt_num + 2]) : (op_dest = mem[REGS_BASE + 2 * REG_CS], op_result = mem[REGS_BASE + 2 * REG_CS]  = (op_source = *(unsigned char*)&mem[ 4 * interrupt_num + 2])));
        op_dest = read_ram16(REGS_BASE + 2 * REG_CS), write_ram16(REGS_BASE + 2 * REG_CS, op_result = op_source = read_ram16( 4 * interrupt_num + 2));
	//(i_w ? op_dest = *(unsigned short*)&reg_ip, op_result = *(unsigned short*)&reg_ip  = (op_source = *(unsigned short*)& mem[4 * interrupt_num]) : (op_dest = reg_ip, op_result = reg_ip  = (op_source = *(unsigned char*)& mem[4 * interrupt_num])));
        op_dest = *(unsigned short*)&reg_ip, op_result = *(unsigned short*)&reg_ip  = (op_source = read_ram16(4 * interrupt_num));

	//return regs8[FLAG_TF] = regs8[FLAG_IF] = 0;
        write_regs8(FLAG_TF, 0);
        write_regs8(FLAG_IF, 0);
        return 0;
}

// AAA and AAS instructions - which_operation is +1 for AAA, and -1 for AAS
int AAA_AAS(char which_operation)
{
	//return (regs16[REG_AX] += 262 * which_operation*set_AF(set_CF(((regs8[REG_AL] & 0x0F) > 9) || regs8[FLAG_AF])), regs8[REG_AL] &= 0x0F);
        uint8_t ax = read_regs16(REG_AX);
        ax += 262 * which_operation*set_AF(set_CF(((ax & 0x0F) > 9) || read_regs8(FLAG_AF)));
        ax &= 0xff0f;
        write_regs16(REG_AX, ax);
        return ax & 0x0f;
}

#if 0
void audio_callback(void *data, unsigned char *stream, int len)
{
	for (int i = 0; i < len; i++)
		stream[i] = (spkr_en == 3) && *(unsigned short*)&mem[0x4AA] ? -((54 * wave_counter++ / *(unsigned short*)&mem[0x4AA]) & 1) : sdl_audio.silence;

	spkr_en = io_ports[0x61] & 3;
}
#endif

// Emulator entry point
int main(int argc, char **argv)
{
logfd=stdout;//fopen("log","wb");
ram_init();
#if 0
	// Initialise SDL
	SDL_Init(SDL_INIT_AUDIO);
	sdl_audio.callback = audio_callback;



	SDL_OpenAudio(&sdl_audio, 0);
#endif

	// regs16 and reg8 point to F000:0, the start of memory-mapped registers. CS is initialised to F000
	//regs16 = (unsigned short *)(regs8 = mem + REGS_BASE);
	//regs16[REG_CS] = 0xF000;
        write_regs16( REG_CS, 0xf000 );
//printf("cs %04x\n",read_regs16(REG_CS));

	// Trap flag off
	//regs8[FLAG_TF] = 0;
        write_regs8( FLAG_TF, 0 );

	// Set DL equal to the boot device: 0 for the FD, or 0x80 for the HD. Normally, boot from the FD.
	// But, if the HD image file is prefixed with @, then boot from the HD
	//regs8[REG_DL] = ((argc > 3) && (*argv[3] == '@')) ? argv[3]++, 0x80 : 0;
        write_regs8( REG_DL, ((argc > 3) && (*argv[3] == '@')) ? argv[3]++, 0x80 : 0 );

	// Open BIOS (file id disk[2]), floppy disk image (disk[1]), and hard disk image (disk[0]) if specified
#if 0
	for (file_index = 3; file_index;)
		disk[--file_index] = *++argv ? open(*argv, 32898) : 0;
#endif

	// Set CX:AX equal to the hard disk image size, if present
	//*(unsigned*)&regs16[REG_AX] = *disk ? lseek(*disk, 0, 2) >> 9 : 0;
        uint32_t hd_size = 0;
        write_regs16( REG_AX, hd_size );
        write_regs16( REG_CX, hd_size >> 16 );

        write_regs16(REG_BX, 0 );
        write_regs16(REG_DX, 0 );
        write_regs16(REG_DI, 0 );
        write_regs16(REG_SI, 0 );
        write_regs16(REG_SP, 0 );
        write_regs16(REG_BP, 0 );
        write_regs16(REG_DS, 0 );
        write_regs16(REG_SS, 0 );
        write_regs16(REG_ES, 0 );

	// Load BIOS image into F000:0100, and set IP to 0100
	//read(disk[2], regs8 + (reg_ip = 0x100), 0xFF00);
reg_ip = 0x100;
#if 0
disk[2]=open("bios",32898);
        for( uint32_t i = 0; i < 0xff00; i++ ) {
          uint8_t temp;
          read( disk[2], &temp, 1 );
          write_ram8( 0xf0100 + i, temp );
        }

        // load floppy image
disk[0]=open("fd.img",32898);
        for( int32_t i = 0; i < 1 << 21; i++ ) {
          uint8_t temp;
          read( disk[0], &temp, 1 );
          write_disk( 0, i, temp );
        }
#endif
	// Load instruction decoding helper table
#if 0
	for (int i = 0; i < 20; i++)
		for (int j = 0; j < 256; j++)
			bios_table_lookup[i][j] = regs8[regs16[0x81 + i] + j];
#endif
//logfd=fopen("log","wb");
	// Instruction execution loop. Terminates if CS:IP = 0:0
	//for (; opcode_stream = mem + 16 * regs16[REG_CS] + reg_ip, opcode_stream != mem;)
        //for( ; opcode_stream = mem + 16L * read_regs16(REG_CS) + reg_ip, opcode_stream != mem;)
        for( ;opcode_stream = 16L * read_regs16(REG_CS) + reg_ip; )
	{
                opcode_stream = 16L * read_regs16(REG_CS) + reg_ip;
#if !defined(C64) && !defined(C128)
                if( !opcode_stream) break;
#endif
//printf("inst_counter %d opcode_stream %05x cs %04x reg_ip %04x dl %d opcode %02x\n",inst_counter, opcode_stream, read_regs16(REG_CS), reg_ip, read_regs8(REG_DL), read_ram8(opcode_stream));
//if(inst_counter==/*66000*/1200000)return 0;
//if(inst_counter>=/*973870*/ /*972880*/0)
//fprintf(logfd,"%ld %05lx %02x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %04x %02x\n",inst_counter, opcode_stream, read_ram8( opcode_stream), read_regs16(REG_AX), read_regs16(REG_BX), read_regs16(REG_CX), read_regs16(REG_DX), read_regs16(REG_DI), read_regs16(REG_SI), read_regs16(REG_SP), read_regs16(REG_BP), read_regs16(REG_CS), read_regs16(REG_DS), read_regs16(REG_SS), read_regs16(REG_ES), read_ram8(0x11040));
if(inst_counter==973875){
//printf("%02x %02x %02x %02x %02x %05x %02x\n", read_ram8(opcode_stream), read_ram8(opcode_stream+1), read_ram8(opcode_stream+2), read_ram8(opcode_stream+3), read_ram8( read_regs16(REG_SI) + 16L*read_regs16(REG_DS)), read_regs16(REG_SI) + 16L*read_regs16(REG_DS), read_ram8(0x11040));
//__asm__ volatile(".byte 0xcc");
}
//if(inst_counter>=/*973880*/83803){fclose(logfd);puts("DONE");for(;;);}
//if(inst_counter%1000==0)printf("inst_counter %d\r\n",inst_counter);
		// Set up variables to prepare for decoding an opcode
		//set_opcode(*opcode_stream);
                set_opcode( read_ram8( opcode_stream ) );

		// Extract i_w and i_d fields from instruction
		i_w = (i_reg4bit = raw_opcode_id & 7) & 1;
		i_d = i_reg4bit / 2 & 1;

		// Extract instruction data fields
		//i_data0 = *(short*)&opcode_stream[1];
                i_data0 = read_ram16( opcode_stream + 1 );
		//i_data1 = *(short*)&opcode_stream[2];
                i_data1 = read_ram16( opcode_stream + 2 );
		//i_data2 = *(short*)&opcode_stream[3];
                i_data2 = read_ram16( opcode_stream + 3 );

		// seg_override_en and rep_override_en contain number of instructions to hold segment override and REP prefix respectively
		if (seg_override_en)
			seg_override_en--;
		if (rep_override_en)
			rep_override_en--;

		// i_mod_size > 0 indicates that opcode uses i_mod/i_rm/i_reg, so decode them
		if (i_mod_size)
		{
			i_mod = (i_data0 & 0xFF) >> 6;
			i_rm = i_data0 & 7;
			i_reg = i_data0 / 8 & 7;

			if ((!i_mod && i_rm == 6) || (i_mod == 2))
				//i_data2 = *(short*)&opcode_stream[4];
                                i_data2 = read_ram16( opcode_stream + 4 );
			else if (i_mod != 1)
				i_data2 = i_data1;
			else // If i_mod is 1, operand is (usually) 8 bits rather than 16 bits
				i_data1 = (int8_t)i_data1;

			//scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16 * regs16[seg_override_en ? seg_override : bios_table_lookup[scratch2_uint + 3][i_rm]] + (unsigned short)(regs16[bios_table_lookup[scratch2_uint + 1][i_rm]] + bios_table_lookup[scratch2_uint + 2][i_rm] * i_data1+ regs16[bios_table_lookup[scratch2_uint][i_rm]]) : (REGS_BASE + (i_w ? 2 * i_rm : 2 * i_rm + i_rm / 4 & 7)), op_from_addr = (REGS_BASE + (i_w ? 2 * i_reg : 2 * i_reg + i_reg / 4 & 7)), i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint);
                        scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16L * read_regs16(seg_override_en ? seg_override : read_bios_table_lookup(scratch2_uint + 3, i_rm)) + (unsigned short)(read_regs16(read_bios_table_lookup(scratch2_uint + 1, i_rm)) + read_bios_table_lookup(scratch2_uint + 2, i_rm) * i_data1+ read_regs16(read_bios_table_lookup(scratch2_uint, i_rm))) : (REGS_BASE + (i_w ? 2 * i_rm : 2 * i_rm + i_rm / 4 & 7)), op_from_addr = (REGS_BASE + (i_w ? 2 * i_reg : 2 * i_reg + i_reg / 4 & 7)), i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint);
		}

		// Instruction execution unit
		switch (xlat_opcode_id)
		{
			; case 0: // Conditional jump (JAE, JNAE, etc.)
				// i_w is the invert flag, e.g. i_w == 1 means JNAE, whereas i_w == 0 means JAE 
				scratch_uchar = raw_opcode_id / 2 & 7;
				//reg_ip += (char)i_data0 * (i_w ^ (regs8[bios_table_lookup[TABLE_COND_JUMP_DECODE_A][scratch_uchar]] || regs8[bios_table_lookup[TABLE_COND_JUMP_DECODE_B][scratch_uchar]] || regs8[bios_table_lookup[TABLE_COND_JUMP_DECODE_C][scratch_uchar]] ^ regs8[bios_table_lookup[TABLE_COND_JUMP_DECODE_D][scratch_uchar]]))
                                reg_ip += (int8_t)i_data0 * (i_w ^ (read_regs8(read_bios_table_lookup(TABLE_COND_JUMP_DECODE_A, scratch_uchar)) || read_regs8(read_bios_table_lookup(TABLE_COND_JUMP_DECODE_B, scratch_uchar)) || read_regs8(read_bios_table_lookup(TABLE_COND_JUMP_DECODE_C, scratch_uchar)) ^ read_regs8(read_bios_table_lookup(TABLE_COND_JUMP_DECODE_D, scratch_uchar))))

			;break; case 1: // MOV reg, imm
				i_w = !!(raw_opcode_id & 8);
				//(i_w ? op_dest = *(unsigned short*)&mem[(REGS_BASE + (i_w ? 2 * i_reg4bit : 2 * i_reg4bit + i_reg4bit / 4 & 7))], op_result = *(unsigned short*)&mem[(REGS_BASE + (i_w ? 2 * i_reg4bit : 2 * i_reg4bit + i_reg4bit / 4 & 7))]  = (op_source = *(unsigned short*)& i_data0) : (op_dest = mem[(REGS_BASE + (i_w ? 2 * i_reg4bit : 2 * i_reg4bit + i_reg4bit / 4 & 7))], op_result = mem[(REGS_BASE + (i_w ? 2 * i_reg4bit : 2 * i_reg4bit + i_reg4bit / 4 & 7))]  = (op_source = *(unsigned char*)& i_data0)))
                                (i_w ? op_dest = read_ram16(REGS_BASE + (2 * i_reg4bit)), write_ram16(REGS_BASE + (2 * i_reg4bit), op_result = op_source = *(unsigned short*)& i_data0) : (op_dest = read_ram8(REGS_BASE + (2 * i_reg4bit + i_reg4bit / 4 & 7)), write_ram8(REGS_BASE + (2 * i_reg4bit + i_reg4bit / 4 & 7), op_result = op_source = *(unsigned char*)& i_data0)))
			;break; case 3: // PUSH regs16
                                {uint16_t sp;
				//(i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&regs16[i_reg4bit]) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&regs16[i_reg4bit]))))
                                (i_w = 1, sp = read_regs16(REG_SP)-1, (op_dest = read_ram8(16L * read_regs16(REG_SS) + sp), sp--, write_ram16(16L * read_regs16(REG_SS) + sp, op_result = op_source = read_regs16(i_reg4bit)) )), write_regs16( REG_SP, sp )
			;}break; case 4: // POP regs16
                                {uint16_t sp;
				//(i_w = 1, regs16[REG_SP] += 2, (i_w ? op_dest = *(unsigned short*)&regs16[i_reg4bit], op_result = *(unsigned short*)&regs16[i_reg4bit] = (op_source = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]) : (op_dest = regs16[i_reg4bit], op_result = regs16[i_reg4bit] = (op_source = *(unsigned char*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]))))
                                (i_w = 1, sp = read_regs16(REG_SP), (op_dest = read_regs16(i_reg4bit), write_regs16(i_reg4bit, op_result = op_source = read_ram16(16L * read_regs16(REG_SS) + sp)) )), sp += 2, write_regs16(REG_SP, sp)
			;}break; case 2: // INC|DEC regs16
				i_w = 1;
				i_d = 0;
				i_reg = i_reg4bit;
				//scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16 * regs16[seg_override_en ? seg_override : bios_table_lookup[scratch2_uint + 3][i_rm]] + (unsigned short)(regs16[bios_table_lookup[scratch2_uint + 1][i_rm]] + bios_table_lookup[scratch2_uint + 2][i_rm] * i_data1+ regs16[bios_table_lookup[scratch2_uint][i_rm]]) : (REGS_BASE + (i_w ? 2 * i_rm : 2 * i_rm + i_rm / 4 & 7)), op_from_addr = (REGS_BASE + (i_w ? 2 * i_reg : 2 * i_reg + i_reg / 4 & 7)), i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint);
                                scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16L * read_regs16(seg_override_en ? seg_override : read_bios_table_lookup(scratch2_uint + 3, i_rm)) + (unsigned short)(read_regs16(read_bios_table_lookup(scratch2_uint + 1, i_rm)) + read_bios_table_lookup(scratch2_uint + 2, i_rm) * i_data1+ read_regs16(read_bios_table_lookup(scratch2_uint, i_rm))) : (REGS_BASE + (2 * i_rm )), op_from_addr = (REGS_BASE + (2 * i_reg ));
				i_reg = extra
			; case 5: // INC|DEC|JMP|CALL|PUSH
				if (i_reg < 2) // INC|DEC
                                        {uint16_t temp;
					//(i_w ? op_dest = *(unsigned short*)&mem[op_from_addr], op_result = *(unsigned short*)&mem[op_from_addr]  += 1 - 2 * i_reg + (op_source = *(unsigned short*)&mem[ REGS_BASE + 2 * REG_ZERO]) : (op_dest = mem[op_from_addr], op_result = mem[op_from_addr]  += 1 - 2 * i_reg + (op_source = *(unsigned char*)&mem[ REGS_BASE + 2 * REG_ZERO]))),
                                        (i_w ? op_dest = read_ram16(op_from_addr), temp = read_ram16(op_from_addr), op_result = temp += 1 - 2 * i_reg + (op_source = read_ram16(REGS_BASE + 2 * REG_ZERO)), write_ram16( op_from_addr, temp) : (op_dest = read_ram8(op_from_addr), temp = read_ram8(op_from_addr), op_result = temp += 1 - 2 * i_reg + (op_source = read_ram16( REGS_BASE + 2 * REG_ZERO)), write_ram8(op_from_addr, temp))),
					op_source = 1,
					set_AF_OF_arith(),
					set_OF(op_dest + 1 - i_reg == 1 << (8*(i_w + 1) - 1)),
					(xlat_opcode_id == 5) && (set_opcode(0x10), 0); // Decode like ADC
                                        }
				else if (i_reg != 6) // JMP|CALL
                                        {uint16_t sp;
					//i_reg - 3 || (i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&regs16[REG_CS]) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&regs16[REG_CS])))), // CALL (far)
                                        i_reg - 3 || (i_w = 1, sp = read_regs16(REG_SP)-1,(op_dest = read_ram16(16L * read_regs16(REG_SS) + sp), sp--, write_ram16(16L * read_regs16(REG_SS) + sp, op_result = op_source = read_regs16(REG_CS)) ), write_regs16(REG_SP, sp), 0), // CALL (far)
					//i_reg & 2 && (i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&reg_ip + 2 + i_mod*(i_mod != 3) + 2*(!i_mod && i_rm == 6)) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&reg_ip + 2 + i_mod*(i_mod != 3) + 2*(!i_mod && i_rm == 6))))), // CALL (near or far)
                                        i_reg & 2 && (i_w = 1, sp = read_regs16(REG_SP)-1,(op_dest = read_ram16(16L * read_regs16(REG_SS) + read_regs16(REG_SP)), sp--, write_ram16(16L * read_regs16(REG_SS) + sp, op_result = op_source = *(unsigned short*)&reg_ip + 2 + i_mod*(i_mod != 3) + 2*(!i_mod && i_rm == 6)) ), write_regs16(REG_SP, sp), 0), // CALL (near or far)
					//i_reg & 1 && (regs16[REG_CS] = *(short*)&mem[op_from_addr + 2]), // JMP|CALL (far)
                                        i_reg & 1 && (write_regs16(REG_CS, read_ram16(op_from_addr + 2)),0), // JMP|CALL (far)
					//(i_w ? op_dest = *(unsigned short*)&reg_ip, op_result = *(unsigned short*)&reg_ip  = (op_source = *(unsigned short*)& mem[op_from_addr]) : (op_dest = reg_ip, op_result = reg_ip  = (op_source = *(unsigned char*)& mem[op_from_addr]))),
                                        (i_w ? op_dest = *(unsigned short*)&reg_ip, op_result = *(unsigned short*)&reg_ip  = (op_source = read_ram16(op_from_addr)) : (op_dest = reg_ip, op_result = reg_ip  = (op_source = read_ram8(op_from_addr)))),
					set_opcode(0x9A); // Decode like CALL
                                        }
				else // PUSH
                                        {uint16_t sp;
					//(i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&mem[rm_addr]) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&mem[rm_addr]))))
                                        (i_w = 1, sp = read_regs16(REG_SP)-1, (op_dest = read_ram16(16L * read_regs16(REG_SS) + sp), sp--, write_ram16(16L * read_regs16(REG_SS) + sp, op_result = op_source = read_ram16(rm_addr)) ), write_regs16(REG_SP, sp))
			;}break; case 6: // TEST r/m, imm16 / NOT|NEG|UL|IMUL|DIV|IDIV reg
				op_to_addr = op_from_addr;

				switch (i_reg)
				{
					; case 0: // TEST
						set_opcode(0x20); // Decode like AND
						reg_ip += i_w + 1;
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr]  & (op_source = *(unsigned short*)& i_data2) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr]  & (op_source = *(unsigned char*)& i_data2)))
                                                (i_w ? op_dest = read_ram16(op_to_addr), op_result = op_dest  & (op_source = *(unsigned short*)& i_data2) : (op_dest = read_ram8(op_to_addr), op_result = op_dest  & (op_source = *(unsigned char*)& i_data2)))
					;break; case 2: // NOT
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] =~ (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] =~ (op_source = *(unsigned char*)&mem[op_from_addr])))
                                                (i_w ? op_dest = read_ram16(op_to_addr), write_ram16(op_to_addr, op_result =~ (op_source = read_ram16(op_from_addr))) : (op_dest = read_ram8(op_to_addr), write_ram8(op_to_addr, op_result =~ (op_source = read_ram8(op_from_addr)))))
					;break; case 3: // NEG
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] =- (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] =- (op_source = *(unsigned char*)&mem[op_from_addr])));
                                                (i_w ? op_dest = read_ram16(op_to_addr), write_ram16(op_to_addr, op_result = -(op_source = read_ram16(op_from_addr))) : (op_dest = read_ram8(op_to_addr), write_ram8(op_to_addr, op_result = -(op_source = read_ram8(op_from_addr)))));
						op_dest = 0;
						set_opcode(0x28); // Decode like SUB
						set_CF(op_result > op_dest)
					;break; case 4: // MUL
						//i_w ? (set_opcode(0x10),  regs16[i_w + 1] = (op_result = *(unsigned short*)&mem[rm_addr] * (unsigned short)* regs16) >> 16, regs16[REG_AX] = op_result, set_OF(set_CF(op_result - (unsigned short)op_result))) : (set_opcode(0x10),  regs8[i_w + 1] = (op_result = *(unsigned char*)&mem[rm_addr] * (unsigned char)* regs8) >> 16, regs16[REG_AX] = op_result, set_OF(set_CF(op_result - (unsigned char)op_result)))
                                                i_w ? (set_opcode(0x10),  write_regs16(i_w + 1, (op_result = (uint32_t)read_ram16(rm_addr) * (uint32_t)read_regs16(REG_AX)) >> 16), write_regs16(REG_AX, op_result), set_OF(set_CF(op_result - (unsigned short)op_result))) : (set_opcode(0x10),  write_regs8(i_w + 1, (op_result = read_ram8(rm_addr) * read_regs8(REG_AL)) >> 16), write_regs16(REG_AX, op_result), set_OF(set_CF(op_result - (unsigned char)op_result)))
					;break; case 5: // IMUL
						//i_w ? (set_opcode(0x10),  regs16[i_w + 1] = (op_result = *(short*)&mem[rm_addr] * (short)* regs16) >> 16, regs16[REG_AX] = op_result, set_OF(set_CF(op_result - (short)op_result))) : (set_opcode(0x10),  regs8[i_w + 1] = (op_result = *(char*)&mem[rm_addr] * (char)* regs8) >> 16, regs16[REG_AX] = op_result, set_OF(set_CF(op_result - (char)op_result)))
                                                i_w ? (set_opcode(0x10),  write_regs16(i_w + 1, (op_result = (int32_t)(int16_t)read_ram16(rm_addr) * (int32_t)(int16_t)read_regs16(REG_AX)) >> 16), write_regs16(REG_AX, op_result), set_OF(set_CF(op_result - (short)op_result))) : (set_opcode(0x10),  write_regs8(i_w + 1, (op_result = (int8_t)read_ram8(rm_addr) * (int8_t)read_regs8(REG_AL)) >> 16), write_regs16(REG_AX, op_result), set_OF(set_CF(op_result - (int8_t)op_result)))
					;break; case 6: // DIV
						//i_w ? (scratch_int = *(unsigned short*)&mem[rm_addr]) && !(scratch2_uint = ( unsigned)(scratch_uint = ( regs16[i_w+1] << 16) + regs16[REG_AX]) / scratch_int, scratch2_uint - (unsigned short)scratch2_uint) ?  regs16[i_w+1] = scratch_uint - scratch_int * (* regs16 = scratch2_uint) : pc_interrupt(0) : (scratch_int = *(unsigned char*)&mem[rm_addr]) && !(scratch2_uint = ( unsigned short)(scratch_uint = ( regs8[i_w+1] << 16) + regs16[REG_AX]) / scratch_int, scratch2_uint - (unsigned char)scratch2_uint) ?  regs8[i_w+1] = scratch_uint - scratch_int * (* regs8 = scratch2_uint) : pc_interrupt(0)
                                                i_w ? (scratch_int = read_ram16(rm_addr)) && !(scratch2_uint = ( uint32_t)(scratch_uint = ( (uint32_t)read_regs16(i_w+1) << 16) + read_regs16(REG_AX)) / scratch_int, scratch2_uint - (unsigned short)scratch2_uint) ? write_regs16(REG_AX, scratch2_uint), write_regs16(i_w+1, scratch_uint - scratch_int * scratch2_uint) : pc_interrupt(0) : (scratch_int = read_ram8(rm_addr)) && !(scratch2_uint = ( unsigned short)(scratch_uint = ( (uint32_t)read_regs8(i_w+1) << 16) + read_regs16(REG_AX)) / scratch_int, scratch2_uint - (unsigned char)scratch2_uint) ? write_regs8(REG_AL, scratch2_uint), write_regs8(i_w+1, scratch_uint - scratch_int * scratch2_uint) : pc_interrupt(0)
					;break; case 7: // IDIV
						//i_w ? (scratch_int = *(short*)&mem[rm_addr]) && !(scratch2_uint = ( int)(scratch_uint = ( regs16[i_w+1] << 16) + regs16[REG_AX]) / scratch_int, scratch2_uint - (short)scratch2_uint) ?  regs16[i_w+1] = scratch_uint - scratch_int * (* regs16 = scratch2_uint) : pc_interrupt(0) : (scratch_int = *(char*)&mem[rm_addr]) && !(scratch2_uint = ( short)(scratch_uint = ( regs8[i_w+1] << 16) + regs16[REG_AX]) / scratch_int, scratch2_uint - (char)scratch2_uint) ?  regs8[i_w+1] = scratch_uint - scratch_int * (* regs8 = scratch2_uint) : pc_interrupt(0);
                                                i_w ? (scratch_int = read_ram16(rm_addr)) && !(scratch2_uint = ( int32_t)(scratch_uint = ( (uint32_t)read_regs16(i_w+1) << 16) + read_regs16(REG_AX)) / scratch_int, scratch2_uint - (short)scratch2_uint) ?  write_regs16( REG_AX, scratch2_uint ), write_regs16(i_w+1, scratch_uint - scratch_int * scratch2_uint) : pc_interrupt(0) : (scratch_int = read_ram8(rm_addr)) && !(scratch2_uint = ( short)(scratch_uint = ( (uint32_t)read_regs8(i_w+1) << 16) + read_regs16(REG_AX)) / scratch_int, scratch2_uint - (int8_t)scratch2_uint) ? write_regs8( REG_AL, scratch2_uint ), write_regs8(i_w+1, scratch_uint - scratch_int * scratch2_uint) : pc_interrupt(0);
				}
			;break; case 7: // ADD|OR|ADC|SBB|AND|SUB|XOR|CMP AL/AX, immed
				rm_addr = REGS_BASE;
				i_data2 = i_data0;
				i_mod = 3;
				i_reg = extra;
				reg_ip--;
			; case 8: // ADD|OR|ADC|SBB|AND|SUB|XOR|CMP reg, immed
				op_to_addr = rm_addr;
				//write_regs16(REG_SCRATCH, (i_d |= !i_w) ? (char)i_data2 : i_data2);
                                write_regs16(REG_SCRATCH, (i_d |= !i_w) ? (int8_t)i_data2 : i_data2);
				op_from_addr = REGS_BASE + 2 * REG_SCRATCH;
				reg_ip += !i_d + 1;
				set_opcode(0x08 * (extra = i_reg));
			; case 9: // ADD|OR|ADC|SBB|AND|SUB|XOR|CMP|MOV reg, r/m
				switch (extra)
				{
					; case 0: // ADD
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] += (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] += (op_source = *(unsigned char*)&mem[op_from_addr]))),
                                                (i_w ? op_dest = read_ram16(op_to_addr), write_ram16(op_to_addr, op_dest + (op_source = read_ram16(op_from_addr))), op_result = read_ram16(op_to_addr) : (op_dest = read_ram8(op_to_addr), write_ram8(op_to_addr, op_dest + (op_source = read_ram8(op_from_addr))), op_result = read_ram8(op_to_addr) )),
						set_CF(op_result < op_dest)
					;break; case 1: // OR
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] |= (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] |= (op_source = *(unsigned char*)&mem[op_from_addr])))
                                                (i_w ? op_dest = read_ram16(op_to_addr), write_ram16(op_to_addr, op_dest | (op_source = read_ram16(op_from_addr))), op_result = read_ram16(op_to_addr) : (op_dest = read_ram8(op_to_addr), write_ram8(op_to_addr, op_dest | (op_source = read_ram8(op_from_addr))), op_result = read_ram8(op_to_addr)))
					;break; case 2: // ADC
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] += regs8[FLAG_CF] + (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] += regs8[FLAG_CF] + (op_source = *(unsigned char*)&mem[op_from_addr]))), set_CF(regs8[FLAG_CF] && (op_result == op_dest) || (+ op_result < +(int)op_dest)), set_AF_OF_arith()
                                                (i_w ? op_dest = read_ram16(op_to_addr), write_ram16( op_to_addr, op_dest + read_regs8(FLAG_CF) + (op_source = read_ram16(op_from_addr))), op_result = read_ram16( op_to_addr ) : (op_dest = read_ram8(op_to_addr), write_ram8( op_to_addr, op_dest + read_regs8(FLAG_CF) + (op_source = read_ram8(op_from_addr))), op_result = read_ram8( op_to_addr ) )), set_CF(read_regs8(FLAG_CF) && (op_result == op_dest) || (+ op_result < +(int32_t)op_dest)), set_AF_OF_arith()
					;break; case 3: // SBB
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] -= regs8[FLAG_CF] + (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] -= regs8[FLAG_CF] + (op_source = *(unsigned char*)&mem[op_from_addr]))), set_CF(regs8[FLAG_CF] && (op_result == op_dest) || (- op_result < -(int)op_dest)), set_AF_OF_arith()
                                                (i_w ? op_dest = read_ram16( op_to_addr ), write_ram16( op_to_addr,  op_dest - read_regs8(FLAG_CF) - (op_source = read_ram16( op_from_addr ))), op_result = read_ram16( op_to_addr ) : (op_dest = read_ram8( op_to_addr ), write_ram8( op_to_addr, op_dest - read_regs8(FLAG_CF) - (op_source = read_ram8( op_from_addr ))), op_result = read_ram8( op_to_addr ))), set_CF(read_regs8(FLAG_CF) && (op_result == op_dest) || (- op_result < -(int32_t)op_dest)), set_AF_OF_arith()
					;break; case 4: // AND
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] &= (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] &= (op_source = *(unsigned char*)&mem[op_from_addr])))
                                                (i_w ? op_dest = read_ram16(op_to_addr), write_ram16( op_to_addr, op_dest & (op_source = read_ram16(op_from_addr))), op_dest = read_ram16( op_to_addr ) : (op_dest = read_ram8(op_to_addr), write_ram8( op_to_addr, op_dest & (op_source = read_ram8(op_from_addr))), op_result = read_ram8( op_to_addr )))
					;break; case 5: // SUB
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] -= (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] -= (op_source = *(unsigned char*)&mem[op_from_addr]))),
                                                (i_w ? op_dest = read_ram16(op_to_addr), write_ram16( op_to_addr, op_dest - (op_source = read_ram16(op_from_addr))), op_result = read_ram16( op_to_addr ) : (op_dest = read_ram8(op_to_addr), write_ram8( op_to_addr, op_dest - (op_source = read_ram8(op_from_addr))), op_result = read_ram8( op_to_addr ))),
						set_CF(op_result > op_dest)
					;break; case 6: // XOR
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] ^= (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] ^= (op_source = *(unsigned char*)&mem[op_from_addr])))
                                                (i_w ? op_dest = read_ram16(op_to_addr), write_ram16( op_to_addr, op_dest ^ (op_source = read_ram16(op_from_addr))), op_result = read_ram16( op_to_addr ) : (op_dest = read_ram8(op_to_addr), write_ram8( op_to_addr, op_dest ^ (op_source = read_ram8(op_from_addr))), op_result = read_ram8( op_to_addr )))
					;break; case 7: // CMP
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] - (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] - (op_source = *(unsigned char*)&mem[op_from_addr]))),
                                                (i_w ? op_dest = read_ram16(op_to_addr), op_result = op_dest - (op_source = read_ram16(op_from_addr)) : (op_dest = read_ram8(op_to_addr), op_result = op_dest - (op_source = read_ram8(op_from_addr)))),
						set_CF(op_result > op_dest)
					;break; case 8: // MOV
						//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] = (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] = (op_source = *(unsigned char*)&mem[op_from_addr])));
                                                (i_w ? op_dest = read_ram16(op_to_addr), write_ram16(op_to_addr, op_source = read_ram16(op_from_addr)), op_result = read_ram16(op_to_addr) : (op_dest = read_ram8(op_to_addr), write_ram8(op_to_addr, op_source = read_ram8(op_from_addr)), op_result = read_ram8(op_to_addr)));
				}
			;break; case 10: // MOV sreg, r/m | POP r/m | LEA reg, r/m
				if (!i_w) // MOV
					i_w = 1,
					i_reg += 8,
					//scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16 * regs16[seg_override_en ? seg_override : bios_table_lookup[scratch2_uint + 3][i_rm]] + (unsigned short)(regs16[bios_table_lookup[scratch2_uint + 1][i_rm]] + bios_table_lookup[scratch2_uint + 2][i_rm] * i_data1+ regs16[bios_table_lookup[scratch2_uint][i_rm]]) : (REGS_BASE + (i_w ? 2 * i_rm : 2 * i_rm + i_rm / 4 & 7)), op_from_addr = (REGS_BASE + (i_w ? 2 * i_reg : 2 * i_reg + i_reg / 4 & 7)), i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint),
                                        scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16L * read_regs16(seg_override_en ? seg_override : read_bios_table_lookup(scratch2_uint + 3,i_rm)) + (unsigned short)(read_regs16(read_bios_table_lookup(scratch2_uint + 1,i_rm)) + read_bios_table_lookup(scratch2_uint + 2,i_rm) * i_data1+ read_regs16(read_bios_table_lookup(scratch2_uint,i_rm))) : (REGS_BASE + (i_w ? 2 * i_rm : 2 * i_rm + i_rm / 4 & 7)), op_from_addr = (REGS_BASE + (i_w ? 2 * i_reg : 2 * i_reg + i_reg / 4 & 7)), i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint),
					//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] = (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] = (op_source = *(unsigned char*)&mem[op_from_addr])));
                                        (i_w ? op_dest = read_ram16(op_to_addr), write_ram16(op_to_addr, op_source = read_ram16(op_from_addr)), op_result = read_ram16(op_to_addr) : (op_dest = read_ram8(op_to_addr), write_ram8(op_to_addr, op_source = read_ram8(op_from_addr)), op_result = read_ram8(op_to_addr)));
				else if (!i_d) // LEA
					seg_override_en = 1,
					seg_override = REG_ZERO,
					//scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16 * regs16[seg_override_en ? seg_override : bios_table_lookup[scratch2_uint + 3][i_rm]] + (unsigned short)(regs16[bios_table_lookup[scratch2_uint + 1][i_rm]] + bios_table_lookup[scratch2_uint + 2][i_rm] * i_data1+ regs16[bios_table_lookup[scratch2_uint][i_rm]]) : (REGS_BASE + (i_w ? 2 * i_rm : 2 * i_rm + i_rm / 4 & 7)), op_from_addr = (REGS_BASE + (i_w ? 2 * i_reg : 2 * i_reg + i_reg / 4 & 7)), i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint),
                                        scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16L * read_regs16(seg_override_en ? seg_override : read_bios_table_lookup(scratch2_uint + 3,i_rm)) + (unsigned short)(read_regs16(read_bios_table_lookup(scratch2_uint + 1,i_rm)) + read_bios_table_lookup(scratch2_uint + 2,i_rm) * i_data1+ read_regs16(read_bios_table_lookup(scratch2_uint,i_rm))) : (REGS_BASE + (i_w ? 2 * i_rm : 2 * i_rm + i_rm / 4 & 7)), op_from_addr = (REGS_BASE + (i_w ? 2 * i_reg : 2 * i_reg + i_reg / 4 & 7)), i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint),
					//(i_w ? op_dest = *(unsigned short*)&mem[op_from_addr], op_result = *(unsigned short*)&mem[op_from_addr]  = (op_source = *(unsigned short*)& rm_addr) : (op_dest = mem[op_from_addr], op_result = mem[op_from_addr]  = (op_source = *(unsigned char*)& rm_addr)));
                                        (i_w ? op_dest = read_ram16(op_from_addr), write_ram16(op_from_addr, op_source = *(unsigned short*)& rm_addr), op_result = read_ram16(op_from_addr) : (op_dest = read_ram8(op_from_addr), write_ram8(op_from_addr, op_source = *(unsigned char*)& rm_addr), op_result = read_ram8(op_from_addr)));
				else // POP
					//(i_w = 1, regs16[REG_SP] += 2, (i_w ? op_dest = *(unsigned short*)&mem[rm_addr], op_result = *(unsigned short*)&mem[rm_addr] = (op_source = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]) : (op_dest = mem[rm_addr], op_result = mem[rm_addr] = (op_source = *(unsigned char*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]))))
                                        (write_regs16(REG_SP,read_regs16(REG_SP)+2), (op_dest = read_ram16(rm_addr), write_ram16(rm_addr, op_source = read_ram16(16L * read_regs16(REG_SS) + (unsigned short)(-2+ read_regs16(REG_SP)))), op_result = read_ram16(rm_addr) ))
			;break; case 11: // MOV AL/AX, [loc]
				i_mod = i_reg = 0;
				i_rm = 6;
				i_data1 = i_data0;
				//scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16 * regs16[seg_override_en ? seg_override : bios_table_lookup[scratch2_uint + 3][i_rm]] + (unsigned short)(regs16[bios_table_lookup[scratch2_uint + 1][i_rm]] + bios_table_lookup[scratch2_uint + 2][i_rm] * i_data1+ regs16[bios_table_lookup[scratch2_uint][i_rm]]) : (REGS_BASE + (i_w ? 2 * i_rm : 2 * i_rm + i_rm / 4 & 7)), op_from_addr = (REGS_BASE + (i_w ? 2 * i_reg : 2 * i_reg + i_reg / 4 & 7)), i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint);
                                scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16L * read_regs16(seg_override_en ? seg_override : read_bios_table_lookup(scratch2_uint + 3,i_rm)) + (unsigned short)(read_regs16(read_bios_table_lookup(scratch2_uint + 1,i_rm)) + read_bios_table_lookup(scratch2_uint + 2,i_rm) * i_data1+ read_regs16(read_bios_table_lookup(scratch2_uint,i_rm))) : (REGS_BASE + (i_w ? 2 * i_rm : 2 * i_rm + i_rm / 4 & 7)), op_from_addr = (REGS_BASE + (i_w ? 2 * i_reg : 2 * i_reg + i_reg / 4 & 7)), i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint);
				//(i_w ? op_dest = *(unsigned short*)&mem[op_from_addr], op_result = *(unsigned short*)&mem[op_from_addr]  = (op_source = *(unsigned short*)&mem[ op_to_addr]) : (op_dest = mem[op_from_addr], op_result = mem[op_from_addr]  = (op_source = *(unsigned char*)&mem[ op_to_addr])))
                                (i_w ? op_dest = read_ram16(op_from_addr), write_ram16(op_from_addr, op_source = read_ram16( op_to_addr)), op_result = read_ram16(op_from_addr) : (op_dest = read_ram8(op_from_addr), write_ram8(op_from_addr, op_source = read_ram8( op_to_addr)), op_result = read_ram8(op_from_addr)))
			;break; case 12: // ROL|ROR|RCL|RCR|SHL|SHR|???|SAR reg/mem, 1/CL/imm (80186)
				//scratch2_uint = (1 & (i_w ? *(short*)&mem[rm_addr] : mem[rm_addr]) >> (8*(i_w + 1) - 1)),
                                scratch2_uint = (1 & (i_w ? read_ram16(rm_addr) : read_ram8(rm_addr)) >> (8*(i_w + 1) - 1)),
				scratch_uint = extra ? // xxx reg/mem, imm
					++reg_ip,
					//(char)i_data1
                                        (int8_t)i_data1
				: // xxx reg/mem, CL
					i_d
						//? 31 & regs8[REG_CL]
                                                ? 31 & read_regs8(REG_CL)
				: // xxx reg/mem, 1
					1;
				if (scratch_uint)
				{
					if (i_reg < 4) // Rotate operations
						scratch_uint %= i_reg / 2 + 8*(i_w + 1),
						//(i_w ? op_dest = *(unsigned short*)&scratch2_uint, op_result = *(unsigned short*)&scratch2_uint  = (op_source = *(unsigned short*)& mem[rm_addr]) : (op_dest = scratch2_uint, op_result = scratch2_uint  = (op_source = *(unsigned char*)& mem[rm_addr])));
                                                (i_w ? op_dest = *(unsigned short*)&scratch2_uint, op_result = *(unsigned short*)&scratch2_uint  = (op_source = read_ram16(rm_addr)) : (op_dest = scratch2_uint, op_result = scratch2_uint  = (op_source = read_ram8(rm_addr))));
					if (i_reg & 1) // Rotate/shift right operations
						//(i_w ? op_dest = *(unsigned short*)&mem[rm_addr], op_result = *(unsigned short*)&mem[rm_addr]  >>= (op_source = *(unsigned short*)& scratch_uint) : (op_dest = mem[rm_addr], op_result = mem[rm_addr]  >>= (op_source = *(unsigned char*)& scratch_uint)));
                                                (i_w ? op_dest = read_ram16(rm_addr), write_ram16(rm_addr, op_dest >> (op_source = *(unsigned short*)& scratch_uint)), op_result = read_ram16(rm_addr) : (op_dest = read_ram8(rm_addr), write_ram8( rm_addr, op_dest >> (op_source = *(unsigned char*)& scratch_uint)), op_result = read_ram8( rm_addr )));
					else // Rotate/shift left operations
						//(i_w ? op_dest = *(unsigned short*)&mem[rm_addr], op_result = *(unsigned short*)&mem[rm_addr]  <<= (op_source = *(unsigned short*)& scratch_uint) : (op_dest = mem[rm_addr], op_result = mem[rm_addr]  <<= (op_source = *(unsigned char*)& scratch_uint)));
                                                (i_w ? op_dest = read_ram16(rm_addr), write_ram16( rm_addr,  op_dest << (op_source = *(unsigned short*)& scratch_uint)), op_result = read_ram16( rm_addr ) : (op_dest = read_ram8(rm_addr), write_ram8( rm_addr, op_dest << (op_source = *(unsigned char*)& scratch_uint)), op_result = read_ram8( rm_addr )));
					if (i_reg > 3) // Shift operations
						set_opcode(0x10); // Decode like ADC
					if (i_reg > 4) // SHR or SAR
						set_CF(op_dest >> (scratch_uint - 1) & 1);
				}

				switch (i_reg)
				{
					; case 0: // ROL
						//(i_w ? op_dest = *(unsigned short*)&mem[rm_addr], op_result = *(unsigned short*)&mem[rm_addr]  +=  (op_source = *(unsigned short*)& scratch2_uint >> (8*(i_w + 1) - scratch_uint)) : (op_dest = mem[rm_addr], op_result = mem[rm_addr]  +=  (op_source = *(unsigned char*)& scratch2_uint >> (8*(i_w + 1) - scratch_uint))));
                                                (i_w ? op_dest = read_ram16(rm_addr), write_ram16( rm_addr, op_dest + (op_source = *(unsigned short*)& scratch2_uint >> (8*(i_w + 1) - scratch_uint))), op_result = read_ram16( rm_addr ) : (op_dest = read_ram8(rm_addr), write_ram8( rm_addr, op_dest + (op_source = *(unsigned char*)& scratch2_uint >> (8*(i_w + 1) - scratch_uint))), op_result = read_ram8( rm_addr )));
						set_OF((1 & (i_w ? *(short*)&op_result : op_result) >> (8*(i_w + 1) - 1)) ^ set_CF(op_result & 1))
					;break; case 1: // ROR
						scratch2_uint &= (1 << scratch_uint) - 1,
						//(i_w ? op_dest = *(unsigned short*)&mem[rm_addr], op_result = *(unsigned short*)&mem[rm_addr]  +=  (op_source = *(unsigned short*)& scratch2_uint << (8*(i_w + 1) - scratch_uint)) : (op_dest = mem[rm_addr], op_result = mem[rm_addr]  +=  (op_source = *(unsigned char*)& scratch2_uint << (8*(i_w + 1) - scratch_uint))));
                                                (i_w ? op_dest = read_ram16(rm_addr), write_ram16( rm_addr, op_dest + (op_source = *(unsigned short*)& scratch2_uint << (8*(i_w + 1) - scratch_uint))), op_result = read_ram16( rm_addr ) : (op_dest = read_ram8(rm_addr), write_ram8( rm_addr, op_dest + (op_source = *(unsigned char*)& scratch2_uint << (8*(i_w + 1) - scratch_uint))), op_result = read_ram8( rm_addr )));
						set_OF((1 & (i_w ? *(short*)&op_result * 2 : op_result * 2) >> (8*(i_w + 1) - 1)) ^ set_CF((1 & (i_w ? *(short*)&op_result : op_result) >> (8*(i_w + 1) - 1))))
					;break; case 2: // RCL
						//(i_w ? op_dest = *(unsigned short*)&mem[rm_addr], op_result = *(unsigned short*)&mem[rm_addr]  += (regs8[FLAG_CF] << (scratch_uint - 1)) +  (op_source = *(unsigned short*)& scratch2_uint >> (1 + 8*(i_w + 1) - scratch_uint)) : (op_dest = mem[rm_addr], op_result = mem[rm_addr]  += (regs8[FLAG_CF] << (scratch_uint - 1)) +  (op_source = *(unsigned char*)& scratch2_uint >> (1 + 8*(i_w + 1) - scratch_uint))));
                                                (i_w ? op_dest = read_ram16(rm_addr), write_ram16( rm_addr, op_dest  + (read_regs8(FLAG_CF) << (scratch_uint - 1)) +  (op_source = *(unsigned short*)& scratch2_uint >> (1 + 8*(i_w + 1) - scratch_uint))), op_result = read_ram16( rm_addr ) : (op_dest = read_ram8(rm_addr), write_ram8( rm_addr, op_dest + (read_regs8(FLAG_CF) << (scratch_uint - 1)) +  (op_source = *(unsigned char*)& scratch2_uint >> (1 + 8*(i_w + 1) - scratch_uint))), op_result = read_ram8( rm_addr )));
						set_OF((1 & (i_w ? *(short*)&op_result : op_result) >> (8*(i_w + 1) - 1)) ^ set_CF(scratch2_uint & 1 << (8*(i_w + 1) - scratch_uint)))
					;break; case 3: // RCR
						//(i_w ? op_dest = *(unsigned short*)&mem[rm_addr], op_result = *(unsigned short*)&mem[rm_addr]  += (regs8[FLAG_CF] << (8*(i_w + 1) - scratch_uint)) +  (op_source = *(unsigned short*)& scratch2_uint << (1 + 8*(i_w + 1) - scratch_uint)) : (op_dest = mem[rm_addr], op_result = mem[rm_addr]  += (regs8[FLAG_CF] << (8*(i_w + 1) - scratch_uint)) +  (op_source = *(unsigned char*)& scratch2_uint << (1 + 8*(i_w + 1) - scratch_uint))));
                                                (i_w ? op_dest = read_ram16(rm_addr), write_ram16( rm_addr, op_dest + (read_regs8(FLAG_CF) << (8*(i_w + 1) - scratch_uint)) +  (op_source = *(unsigned short*)& scratch2_uint << (1 + 8*(i_w + 1) - scratch_uint))), op_result = read_ram16( rm_addr ) : (op_dest = read_ram8(rm_addr), write_ram8( rm_addr, op_dest + (read_regs8(FLAG_CF) << (8*(i_w + 1) - scratch_uint)) +  (op_source = *(unsigned char*)& scratch2_uint << (1 + 8*(i_w + 1) - scratch_uint))), op_result = read_ram8( rm_addr )));
						set_CF(scratch2_uint & 1 << (scratch_uint - 1));
						set_OF((1 & (i_w ? *(short*)&op_result : op_result) >> (8*(i_w + 1) - 1)) ^ (1 & (i_w ? *(short*)&op_result * 2 : op_result * 2) >> (8*(i_w + 1) - 1)))
					;break; case 4: // SHL
						set_OF((1 & (i_w ? *(short*)&op_result : op_result) >> (8*(i_w + 1) - 1)) ^ set_CF((1 & (i_w ? *(short*)&op_dest << (scratch_uint - 1) : op_dest << (scratch_uint - 1)) >> (8*(i_w + 1) - 1))))
					;break; case 5: // SHR
						set_OF((1 & (i_w ? *(short*)&op_dest : op_dest) >> (8*(i_w + 1) - 1)))
					;break; case 7: // SAR
						scratch_uint < 8*(i_w + 1) || set_CF(scratch2_uint);
						set_OF(0);
						//(i_w ? op_dest = *(unsigned short*)&mem[rm_addr], op_result = *(unsigned short*)&mem[rm_addr]  += (op_source = *(unsigned short*)& scratch2_uint *= ~(((1 << 8*(i_w + 1)) - 1) >> scratch_uint)) : (op_dest = mem[rm_addr], op_result = mem[rm_addr]  += (op_source = *(unsigned char*)& scratch2_uint *= ~(((1 << 8*(i_w + 1)) - 1) >> scratch_uint))));
                                                (i_w ? op_dest = read_ram16(rm_addr), write_ram16( rm_addr, op_dest  + (op_source = *(unsigned short*)& scratch2_uint *= ~(((1 << 8*(i_w + 1)) - 1) >> scratch_uint))), op_result = read_ram16( rm_addr ) : (op_dest = read_ram8(rm_addr), write_ram8(rm_addr, op_dest + (op_source = *(unsigned char*)& scratch2_uint *= ~(((1 << 8*(i_w + 1)) - 1) >> scratch_uint))), op_result = read_ram8(rm_addr)));
				}
			;break; case 13: // LOOPxx|JCZX
				//scratch_uint = !!--regs16[REG_CX];
                                write_regs16(REG_CX, read_regs16(REG_CX) - 1 ), scratch_uint = !!read_regs16(REG_CX);

				switch(i_reg4bit)
				{
					; case 0: // LOOPNZ
						//scratch_uint &= !regs8[FLAG_ZF]
                                                scratch_uint &= !read_regs8(FLAG_ZF);
					;break; case 1: // LOOPZ
						//scratch_uint &= regs8[FLAG_ZF]
                                                scratch_uint &= read_regs8(FLAG_ZF);
					;break; case 3: // JCXXZ
						//scratch_uint = !++regs16[REG_CX];
                                                write_regs16(REG_CX, read_regs16(REG_CX) + 1 ), scratch_uint = !read_regs16(REG_CX);
				}
				//reg_ip += scratch_uint*(char)i_data0
                                reg_ip += scratch_uint*(int8_t)i_data0
			;break; case 14: // JMP | CALL short/near
				reg_ip += 3 - i_d;
				if (!i_w)
				{
					if (i_d) // JMP far
						reg_ip = 0,
						//regs16[REG_CS] = i_data2;
                                                write_regs16(REG_CS, i_data2 );
					else // CALL
						//(i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&reg_ip) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&reg_ip))));
                                                (i_w = 1, write_regs16(REG_SP, read_regs16( REG_SP ) - 1 ), op_dest = read_ram16(16L * read_regs16(REG_SS) + read_regs16(REG_SP)), write_regs16(REG_SP, read_regs16( REG_SP ) - 1 ), write_ram16(op_result = 16L * read_regs16(REG_SS) + read_regs16(REG_SP), op_source = *(unsigned short*)&reg_ip), op_result = read_ram16( op_result ) );
				}
				//reg_ip += i_d && i_w ? (char)i_data0 : i_data0
                                reg_ip += i_d && i_w ? (int8_t)i_data0 : i_data0
			;break; case 15: // TEST reg, r/m
				//(i_w ? op_dest = *(unsigned short*)&mem[op_from_addr], op_result = *(unsigned short*)&mem[op_from_addr]  & (op_source = *(unsigned short*)&mem[ op_to_addr]) : (op_dest = mem[op_from_addr], op_result = mem[op_from_addr]  & (op_source = *(unsigned char*)&mem[ op_to_addr])))
                                (i_w ? op_dest = read_ram16(op_from_addr), write_ram16( op_from_addr, op_dest & (op_source = read_ram16( op_to_addr))), op_result = read_ram16( op_from_addr ) : (op_dest = read_ram8(op_from_addr), write_ram8( op_from_addr, op_dest & (op_source = read_ram8( op_to_addr))), op_result = read_ram8( op_from_addr )))
			;break; case 16: // XCHG AX, regs16
				i_w = 1;
				op_to_addr = REGS_BASE;
				op_from_addr = (REGS_BASE + (i_w ? 2 * i_reg4bit : 2 * i_reg4bit + i_reg4bit / 4 & 7));
			; case 24: // NOP|XCHG reg, r/m
				if (op_to_addr != op_from_addr)
					//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] ^= (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] ^= (op_source = *(unsigned char*)&mem[op_from_addr]))),
                                        (i_w ? op_dest = read_ram16(op_to_addr), write_ram16( op_to_addr, op_dest ^ (op_source = read_ram16(op_from_addr))), op_result = read_ram16( op_to_addr ) : (op_dest = read_ram8(op_to_addr), write_ram8( op_to_addr, op_dest ^ (op_source = read_ram8(op_from_addr))), op_result = read_ram8( op_to_addr ))),
					//(i_w ? op_dest = *(unsigned short*)&mem[op_from_addr], op_result = *(unsigned short*)&mem[op_from_addr]  ^= (op_source = *(unsigned short*)&mem[ op_to_addr]) : (op_dest = mem[op_from_addr], op_result = mem[op_from_addr]  ^= (op_source = *(unsigned char*)&mem[ op_to_addr]))),
                                        (i_w ? op_dest = read_ram16(op_from_addr), write_ram16( op_from_addr, op_dest ^ (op_source = read_ram16( op_to_addr))), op_result = read_ram16( op_from_addr) : (op_dest = read_ram8(op_from_addr), write_ram8( op_from_addr, op_dest  ^ (op_source = read_ram8( op_to_addr))), op_result = read_ram8( op_from_addr ))),
					//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] ^= (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] ^= (op_source = *(unsigned char*)&mem[op_from_addr])))
                                        (i_w ? op_dest = read_ram16(op_to_addr), write_ram16( op_to_addr, op_dest ^ (op_source = read_ram16(op_from_addr))), op_result = read_ram16( op_to_addr ) : (op_dest = read_ram8(op_to_addr), write_ram8( op_to_addr, op_dest ^ (op_source = read_ram8(op_from_addr))), op_result = read_ram8( op_to_addr )))
			;break; case 17: // MOVSx (extra=0)|STOSx (extra=1)|LODSx (extra=2)
				scratch2_uint = seg_override_en ? seg_override : REG_DS;

				//for (scratch_uint = rep_override_en ? regs16[REG_CX] : 1; scratch_uint; scratch_uint--)
                                for (scratch_uint = rep_override_en ? read_regs16(REG_CX) : 1; scratch_uint; scratch_uint--)
				{
					//(i_w ? op_dest = *(unsigned short*)&mem[extra < 2 ? 16 * regs16[REG_ES] + (unsigned short)( regs16[ REG_DI]) : REGS_BASE], op_result = *(unsigned short*)&mem[extra < 2 ? 16 * regs16[REG_ES] + (unsigned short)( regs16[ REG_DI]) : REGS_BASE]  = (op_source = *(unsigned short*)&mem[ extra & 1 ? REGS_BASE : 16 * regs16[scratch2_uint] + (unsigned short)( regs16[ REG_SI])]) : (op_dest = mem[extra < 2 ? 16 * regs16[REG_ES] + (unsigned short)( regs16[ REG_DI]) : REGS_BASE], op_result = mem[extra < 2 ? 16 * regs16[REG_ES] + (unsigned short)( regs16[ REG_DI]) : REGS_BASE]  = (op_source = *(unsigned char*)&mem[ extra & 1 ? REGS_BASE : 16 * regs16[scratch2_uint] + (unsigned short)( regs16[ REG_SI])]))),
                                        (i_w ? op_dest = read_ram16(op_result = extra < 2 ? 16L * read_regs16(REG_ES) + (unsigned short)( read_regs16( REG_DI)) : REGS_BASE), write_ram16( op_result, op_source = read_ram16( extra & 1 ? REGS_BASE : 16L * read_regs16(scratch2_uint) + (unsigned short)( read_regs16( REG_SI)))), op_result = read_ram16( op_result ) : (op_dest = read_ram8( op_result = extra < 2 ? 16L * read_regs16(REG_ES) + (unsigned short)( read_regs16( REG_DI)) : REGS_BASE), write_ram8( op_result, op_source = read_ram8( extra & 1 ? REGS_BASE : 16L * read_regs16(scratch2_uint) + (unsigned short)( read_regs16( REG_SI)))), op_result = read_ram8( op_result ))),
					//extra & 1 || (regs16[REG_SI] -= (2 * regs8[FLAG_DF] - 1)*(i_w + 1)),
                                        extra & 1 || (write_regs16( REG_SI, read_regs16(REG_SI) - (2 * read_regs8(FLAG_DF) - 1)*(i_w + 1)),0),
					//extra & 2 || (regs16[REG_DI] -= (2 * regs8[FLAG_DF] - 1)*(i_w + 1));
                                        extra & 2 || (write_regs16( REG_DI, read_regs16(REG_DI) - (2 * read_regs8(FLAG_DF) - 1)*(i_w + 1)),0);
				}

				if (rep_override_en)
					//regs16[REG_CX] = 0
                                        write_regs16( REG_CX, 0 )
			;break; case 18: // CMPSx (extra=0)|SCASx (extra=1)
				scratch2_uint = seg_override_en ? seg_override : REG_DS;

				//if ((scratch_uint = rep_override_en ? regs16[REG_CX] : 1))
                                if ((scratch_uint = rep_override_en ? read_regs16(REG_CX) : 1))
				{
					for (; scratch_uint; rep_override_en || scratch_uint--)
					{
						//(i_w ? op_dest = *(unsigned short*)&mem[extra ? REGS_BASE : 16 * regs16[scratch2_uint] + (unsigned short)( regs16[ REG_SI])], op_result = *(unsigned short*)&mem[extra ? REGS_BASE : 16 * regs16[scratch2_uint] + (unsigned short)( regs16[ REG_SI])]  - (op_source = *(unsigned short*)&mem[ 16 * regs16[REG_ES] + (unsigned short)( regs16[ REG_DI])]) : (op_dest = mem[extra ? REGS_BASE : 16 * regs16[scratch2_uint] + (unsigned short)( regs16[ REG_SI])], op_result = mem[extra ? REGS_BASE : 16 * regs16[scratch2_uint] + (unsigned short)( regs16[ REG_SI])]  - (op_source = *(unsigned char*)&mem[ 16 * regs16[REG_ES] + (unsigned short)( regs16[ REG_DI])]))),
                                                (i_w ? op_dest = read_ram16(extra ? REGS_BASE : 16L * read_regs16(scratch2_uint) + (unsigned short)( read_regs16( REG_SI))), op_result = read_ram16(extra ? REGS_BASE : 16L * read_regs16(scratch2_uint) + (unsigned short)( read_regs16( REG_SI)))  - (op_source = read_ram16( 16L * read_regs16(REG_ES) + (unsigned short)( read_regs16( REG_DI)))) : (op_dest = read_ram8(extra ? REGS_BASE : 16L * read_regs16(scratch2_uint) + (unsigned short)( read_regs16( REG_SI))), op_result = read_ram8(extra ? REGS_BASE : 16L * read_regs16(scratch2_uint) + (unsigned short)( read_regs16( REG_SI)))  - (op_source = read_ram8( 16L * read_regs16(REG_ES) + (unsigned short)( read_regs16( REG_DI)))))),
						//extra || (regs16[REG_SI] -= (2 * regs8[FLAG_DF] - 1)*(i_w + 1)),
                                                extra || (write_regs16(REG_SI,read_regs16(REG_SI) - (2 * read_regs8(FLAG_DF) - 1)*(i_w + 1)),0),
						//(regs16[REG_DI] -= (2 * regs8[FLAG_DF] - 1)*(i_w + 1)), rep_override_en && !(--regs16[REG_CX] && (!op_result == rep_mode)) && (scratch_uint = 0);
                                                write_regs16( REG_DI, read_regs16(REG_DI) - (2 * read_regs8(FLAG_DF) - 1)*(i_w + 1)), rep_override_en && !((write_regs16(REG_CX,read_regs16(REG_CX)-1),read_regs16(REG_CX)) && ((!op_result) == rep_mode)) && (scratch_uint = 0);
					}

					set_flags_type = FLAGS_UPDATE_SZP | FLAGS_UPDATE_AO_ARITH; // Funge to set SZP/AO flags
					set_CF(op_result > op_dest);
				}
			;break; case 19: // RET|RETF|IRET
				i_d = i_w;
				//(i_w = 1, regs16[REG_SP] += 2, (i_w ? op_dest = *(unsigned short*)&reg_ip, op_result = *(unsigned short*)&reg_ip = (op_source = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]) : (op_dest = reg_ip, op_result = reg_ip = (op_source = *(unsigned char*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]))));
                                (i_w = 1, write_regs16( REG_SP, read_regs16( REG_SP ) + 2 ), (op_dest = *(unsigned short*)&reg_ip, op_result = *(unsigned short*)&reg_ip = (op_source = read_ram16(16L * read_regs16(REG_SS) + (unsigned short)(-2+ read_regs16(REG_SP)))) ));
				if (extra) // IRET|RETF|RETF imm16
					//(i_w = 1, regs16[REG_SP] += 2, (i_w ? op_dest = *(unsigned short*)&regs16[REG_CS], op_result = *(unsigned short*)&regs16[REG_CS] = (op_source = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]) : (op_dest = regs16[REG_CS], op_result = regs16[REG_CS] = (op_source = *(unsigned char*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]))));
                                        (i_w = 1, write_regs16( REG_SP, read_regs16( REG_SP ) + 2 ), (op_dest = read_regs16(REG_CS), write_regs16( REG_CS, op_source = read_ram16(16L * read_regs16(REG_SS) + (unsigned short)(-2+ read_regs16(REG_SP)))), op_result = read_regs16( REG_CS ) ));
				if (extra & 2) // IRET
					//set_flags((i_w = 1, regs16[REG_SP] += 2, (i_w ? op_dest = *(unsigned short*)&scratch_uint, op_result = *(unsigned short*)&scratch_uint = (op_source = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]) : (op_dest = scratch_uint, op_result = scratch_uint = (op_source = *(unsigned char*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])])))));
                                        set_flags((i_w = 1, write_regs16( REG_SP, read_regs16( REG_SP ) + 2 ), (op_dest = *(unsigned short*)&scratch_uint, op_result = *(unsigned short*)&scratch_uint = (op_source = read_ram16(16L * read_regs16(REG_SS) + (unsigned short)(-2+ read_regs16(REG_SP)))) )));
				else if (!i_d) // RET|RETF imm16
					//regs16[REG_SP] += i_data0
                                        write_regs16( REG_SP, read_regs16( REG_SP ) + i_data0 )
			;break; case 20: // MOV r/m, immed
				//(i_w ? op_dest = *(unsigned short*)&mem[op_from_addr], op_result = *(unsigned short*)&mem[op_from_addr]  = (op_source = *(unsigned short*)& i_data2) : (op_dest = mem[op_from_addr], op_result = mem[op_from_addr]  = (op_source = *(unsigned char*)& i_data2)))
                                (i_w ? op_dest = read_ram16(op_from_addr), write_ram16( op_from_addr, op_source = *(unsigned short*)& i_data2), op_result = read_ram16( op_from_addr ) : (op_dest = read_ram8(op_from_addr), write_ram8(op_from_addr, op_source = *(unsigned char*)& i_data2), op_result = read_ram8( op_from_addr )))
			;break; case 21: // IN AL/AX, DX/imm8
//printf("BEFORE %02x\n",read_ram8(0x11040));
				//io_ports[0x20] = 0; // PIC EOI
                                write_io_ports8( 0x20, 0 );
//printf("HERE %02x\n",read_ram8(0x11040));
				//io_ports[0x42] = --io_ports[0x40]; // PIT channel 0/2 read placeholder
                                write_io_ports8( 0x40, read_io_ports8( 0x40 ) - 1 );write_io_ports8( 0x42, read_io_ports8( 0x40 ) );
//printf("HERE %02x\n",read_ram8(0x11040));
				//io_ports[0x3DA] ^= 9; // CGA refresh
                                write_io_ports8( 0x3da, read_io_ports8( 0x3da ) ^ 9 );
				//scratch_uint = extra ? regs16[REG_DX] : (unsigned char)i_data0;
                                scratch_uint = extra ? read_regs16(REG_DX) : (unsigned char)i_data0;
//printf("HERE %02x\n",read_ram8(0x11040));
				//scratch_uint == 0x60 && (io_ports[0x64] = 0); // Scancode read flag
                                if( scratch_uint == 0x60 ) write_io_ports8( 0x64, 0 );
				//scratch_uint == 0x3D5 && (io_ports[0x3D4] >> 1 == 7) && (io_ports[0x3D5] = ((mem[0x49E]*80 + mem[0x49D] + *(short*)&mem[0x4AD]) & (io_ports[0x3D4] & 1 ? 0xFF : 0xFF00)) >> (io_ports[0x3D4] & 1 ? 0 : 8)); // CRT cursor position
                                if(scratch_uint == 0x3D5 && (read_io_ports8(0x3D4) >> 1 == 7)) write_io_ports8(0x3D5, ((read_ram8(0x49E)*80L + read_ram8(0x49D) + read_ram16(0x4AD)) & (read_io_ports8(0x3D4) & 1 ? 0xFF : 0xFF00)) >> (read_io_ports8(0x3D4) & 1 ? 0 : 8)); // CRT cursor position
				//(i_w ? op_dest = *(unsigned short*)&regs8[REG_AL], op_result = *(unsigned short*)&regs8[REG_AL]  = (op_source = *(unsigned short*)& io_ports[scratch_uint]) : (op_dest = regs8[REG_AL], op_result = regs8[REG_AL]  = (op_source = *(unsigned char*)& io_ports[scratch_uint])));
                                (i_w ? op_dest = read_regs16(REG_AL), write_regs16( REG_AL, op_source = read_io_ports16(scratch_uint)), op_result = read_regs16( REG_AL ) : (op_dest = read_regs8(REG_AL), write_regs8( REG_AL, op_source = read_io_ports8(scratch_uint)), op_result = read_regs8( REG_AL )));
//printf("AFTER %02x\n",read_ram8(0x11040));
			;break; case 22: // OUT DX/imm8, AL/AX
				//scratch_uint = extra ? regs16[REG_DX] : (unsigned char)i_data0;
                                scratch_uint = extra ? read_regs16(REG_DX) : (unsigned char)i_data0;
				//(i_w ? op_dest = *(unsigned short*)&io_ports[scratch_uint], op_result = *(unsigned short*)&io_ports[scratch_uint]  = (op_source = *(unsigned short*)& regs8[REG_AL]) : (op_dest = io_ports[scratch_uint], op_result = io_ports[scratch_uint]  = (op_source = *(unsigned char*)& regs8[REG_AL])));
                                (i_w ? op_dest = read_io_ports16(scratch_uint), write_io_ports16( scratch_uint, op_source = read_regs16(REG_AL)), op_result = read_io_ports16( scratch_uint ) : (op_dest = read_io_ports8(scratch_uint), write_io_ports8(scratch_uint, op_source = read_regs8(REG_AL)), op_result = read_io_ports8( scratch_uint )));
				//scratch_uint == 0x61 && (io_hi_lo = 0, spkr_en |= regs8[REG_AL] & 3); // Speaker control
                                scratch_uint == 0x61 && (io_hi_lo = 0, spkr_en |= read_regs8(REG_AL) & 3); // Speaker control
				//(scratch_uint == 0x40 || scratch_uint == 0x42) && (io_ports[0x43] & 6) && (mem[0x469 + scratch_uint - (io_hi_lo ^= 1)] = regs8[REG_AL]); // PIT rate programming
                                (scratch_uint == 0x40 || scratch_uint == 0x42) && (read_io_ports8(0x43) & 6) && (write_ram8(0x469 + scratch_uint - (io_hi_lo ^= 1), read_regs8(REG_AL)),read_ram8(0x469 + scratch_uint - (io_hi_lo))); // PIT rate programming
#if 0
				scratch_uint == 0x43 && (io_hi_lo = 0, regs8[REG_AL] >> 6 == 2) && (SDL_PauseAudio((regs8[REG_AL] & 0xF7) != 0xB6), 0); // Speaker enable
#endif
				//scratch_uint == 0x3D5 && (io_ports[0x3D4] >> 1 == 6) && (mem[0x4AD + !(io_ports[0x3D4] & 1)] = regs8[REG_AL]); // CRT video RAM start offset
                                scratch_uint == 0x3D5 && (read_io_ports8(0x3D4) >> 1 == 6) && (write_ram8(0x4AD + !(read_io_ports8(0x3D4) & 1), read_regs8(REG_AL)),read_ram8(0x4AD + !(read_io_ports8(0x3D4) & 1))); // CRT video RAM start offset
				//scratch_uint == 0x3D5 && (io_ports[0x3D4] >> 1 == 7) && (scratch2_uint = ((mem[0x49E]*80 + mem[0x49D] + *(short*)&mem[0x4AD]) & (io_ports[0x3D4] & 1 ? 0xFF00 : 0xFF)) + (regs8[REG_AL] << (io_ports[0x3D4] & 1 ? 0 : 8)) - *(short*)&mem[0x4AD], mem[0x49D] = scratch2_uint % 80, mem[0x49E] = scratch2_uint / 80); // CRT cursor position
                                scratch_uint == 0x3D5 && (read_io_ports8(0x3D4) >> 1 == 7) && (scratch2_uint = ((read_ram8(0x49E)*80 + read_ram8(0x49D) + read_ram16(0x4AD)) & (read_io_ports8(0x3D4) & 1 ? 0xFF00 : 0xFF)) + (read_regs8(REG_AL) << (read_io_ports8(0x3D4) & 1 ? 0 : 8)) - read_ram16(0x4AD), write_ram8(0x49D, scratch2_uint % 80), write_ram8(0x49E, scratch2_uint / 80), 0); // CRT cursor position
				//scratch_uint == 0x3B5 && io_ports[0x3B4] == 1 && (GRAPHICS_X = regs8[REG_AL] * 16); // Hercules resolution reprogramming. Defaults are set in the BIOS
                                scratch_uint == 0x3B5 && read_io_ports8(0x3B4) == 1 && (GRAPHICS_X = read_regs8(REG_AL) * 16); // Hercules resolution reprogramming. Defaults are set in the BIOS
				//scratch_uint == 0x3B5 && io_ports[0x3B4] == 6 && (GRAPHICS_Y = regs8[REG_AL] * 4);
                                scratch_uint == 0x3B5 && read_io_ports8(0x3B4) == 6 && (GRAPHICS_Y = read_regs8(REG_AL) * 4);
			;break; case 23: // REPxx
				rep_override_en = 2;
				rep_mode = i_w;
				seg_override_en && seg_override_en++
			;break; case 25: // PUSH reg
				//(i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&regs16[extra]) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&regs16[extra]))))
                                (i_w = 1, (write_regs16( REG_SP, read_regs16( REG_SP ) - 1 ), op_dest = read_ram16(16L * read_regs16(REG_SS) + (unsigned short)(read_regs16(REG_SP))), write_regs16( REG_SP, read_regs16( REG_SP ) - 1 ), write_ram16(op_result = 16L * read_regs16(REG_SS) + (unsigned short)(read_regs16(REG_SP)),op_source = read_regs16(extra)), op_result = read_ram16( op_result ) ))
			;break; case 26: // POP reg
				//(i_w = 1, regs16[REG_SP] += 2, (i_w ? op_dest = *(unsigned short*)&regs16[extra], op_result = *(unsigned short*)&regs16[extra] = (op_source = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]) : (op_dest = regs16[extra], op_result = regs16[extra] = (op_source = *(unsigned char*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]))))
                                (i_w = 1, write_regs16(REG_SP, read_regs16( REG_SP ) + 2), (op_dest = read_regs16(extra), write_regs16(extra, op_source = read_ram16(16L * read_regs16(REG_SS) + (unsigned short)(-2+ read_regs16(REG_SP)))), op_result = read_regs16(extra) ))
			;break; case 27: // xS: segment overrides
				seg_override_en = 2;
				seg_override = extra;
				rep_override_en && rep_override_en++
			;break; case 28: // DAA/DAS
				i_w = 0;
				//extra ? set_AF((((scratch2_uint = regs8[REG_AL]) & 0x0F) > 9) || regs8[FLAG_AF]) && (op_result = regs8[REG_AL] -= 6, set_CF(regs8[FLAG_CF] || (regs8[REG_AL]  >= scratch2_uint))), set_CF(((( 0xFF & 1 ? scratch2_uint : regs8[REG_AL]) &  0xFF) >  0x99) || regs8[FLAG_CF]) && (op_result = regs8[REG_AL] -= 0x60) : set_AF((((scratch2_uint = regs8[REG_AL]) & 0x0F) > 9) || regs8[FLAG_AF]) && (op_result = regs8[REG_AL] += 6, set_CF(regs8[FLAG_CF] || (regs8[REG_AL]  < scratch2_uint))), set_CF(((( 0xF0 & 1 ? scratch2_uint : regs8[REG_AL]) &  0xF0) >  0x90) || regs8[FLAG_CF]) && (op_result = regs8[REG_AL] += 0x60) // extra = 0 for DAA, 1 for DAS
                                extra ? set_AF((((scratch2_uint = read_regs8(REG_AL)) & 0x0F) > 9) || read_regs8(FLAG_AF)) && (write_regs8(REG_AL, read_regs8(REG_AL) - 6), op_result = read_regs8(REG_AL), set_CF(read_regs8(FLAG_CF) || (read_regs8(REG_AL)  >= scratch2_uint))), set_CF((( scratch2_uint &  0xFF) >  0x99) || read_regs8(FLAG_CF)) && (write_regs8( REG_AL, read_regs8(REG_AL) - 0x60), op_result = read_regs8(REG_AL)) : set_AF((((scratch2_uint = read_regs8(REG_AL)) & 0x0F) > 9) || read_regs8(FLAG_AF)) && (write_regs8(REG_AL, read_regs8(REG_AL) + 6), op_result = read_regs8(REG_AL), set_CF(read_regs8(FLAG_CF) || (read_regs8(REG_AL)  < scratch2_uint))), set_CF(((read_regs8(REG_AL) &  0xF0) >  0x90) || read_regs8(FLAG_CF)) && (write_regs8(REG_AL, read_regs8(REG_AL) + 0x60), op_result = read_regs8(REG_AL)) // extra = 0 for DAA, 1 for DAS
			;break; case 29: // AAA/AAS
				op_result = AAA_AAS(extra - 1)
			;break; case 30: // CBW
				//regs8[REG_AH] = -(1 & (i_w ? *(short*)&regs8[REG_AL] : regs8[REG_AL]) >> (8*(i_w + 1) - 1))
                                write_regs8(REG_AH, -(1 & (i_w ? read_regs16(REG_AL) : read_regs8(REG_AL)) >> (8*(i_w + 1) - 1)))
			;break; case 31: // CWD
                                //regs16[REG_DX] = -(1 & (i_w ? *(short*)&regs16[REG_AX] : regs16[REG_AX]) >> (8*(i_w + 1) - 1))
				write_regs16(REG_DX, -(1 & (read_regs16(REG_AX)) >> (8*(i_w + 1) - 1)))
			;break; case 32: // CALL FAR imm16:imm16
				//(i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&regs16[REG_CS]) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&regs16[REG_CS]))));
                                (i_w = 1, (write_regs16(REG_SP, read_regs16(REG_SP)-1), op_dest = read_ram16(16L * read_regs16(REG_SS) + (unsigned short)(read_regs16(REG_SP))), write_regs16(REG_SP, read_regs16(REG_SP)-1), write_ram16(op_result = 16L * read_regs16(REG_SS) + (unsigned short)(read_regs16(REG_SP)), op_source = read_regs16(REG_CS)), op_result = read_ram16( op_result ) ));
				//(i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&reg_ip + 5) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&reg_ip + 5))));
                                (i_w = 1, (write_regs16(REG_SP, read_regs16(REG_SP)-1), op_dest = read_ram16(16L * read_regs16(REG_SS) + (unsigned short)(read_regs16(REG_SP))), write_regs16(REG_SP, read_regs16(REG_SP)-1), write_ram16( op_result = 16L * read_regs16(REG_SS) + (unsigned short)(read_regs16(REG_SP)), op_source = *(unsigned short*)&reg_ip + 5), op_result = read_ram16( op_result ) ));
				//regs16[REG_CS] = i_data2;
                                write_regs16(REG_CS, i_data2);
				reg_ip = i_data0
			;break; case 33: // PUSHF
				make_flags();
				//(i_w = 1, (i_w ? op_dest = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned short*)&scratch_uint) : (op_dest = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])], op_result = mem[16 * regs16[REG_SS] + (unsigned short)(-- regs16[REG_SP])] = (op_source = *(unsigned char*)&scratch_uint))))
                                (i_w = 1, (write_regs16(REG_SP, read_regs16(REG_SP)-1), op_dest = read_ram16(16L * read_regs16(REG_SS) + (unsigned short)(read_regs16(REG_SP))), write_regs16(REG_SP, read_regs16(REG_SP)-1), write_ram16(op_result = 16L * read_regs16(REG_SS) + (unsigned short)(read_regs16(REG_SP)), op_source = *(unsigned short*)&scratch_uint), op_result = read_ram16( op_result ) ))
			;break; case 34: // POPF
				//set_flags((i_w = 1, regs16[REG_SP] += 2, (i_w ? op_dest = *(unsigned short*)&scratch_uint, op_result = *(unsigned short*)&scratch_uint = (op_source = *(unsigned short*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])]) : (op_dest = scratch_uint, op_result = scratch_uint = (op_source = *(unsigned char*)&mem[16 * regs16[REG_SS] + (unsigned short)(-2+ regs16[REG_SP])])))))
                                set_flags((i_w = 1, write_regs16( REG_SP, read_regs16( REG_SP ) + 2), (op_dest = *(unsigned short*)&scratch_uint, op_result = *(unsigned short*)&scratch_uint = (op_source = read_ram16(16L * read_regs16(REG_SS) + (unsigned short)(-2+ read_regs16(REG_SP)))) )))
			;break; case 35: // SAHF
				make_flags();
				//set_flags((scratch_uint & 0xFF00) + regs8[REG_AH])
                                set_flags((scratch_uint & 0xFF00) + read_regs8(REG_AH))
			;break; case 36: // LAHF
				make_flags(),
				//regs8[REG_AH] = scratch_uint
                                write_regs8(REG_AH, scratch_uint)
			;break; case 37: // LES|LDS reg, r/m
				i_w = i_d = 1;
				//scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16 * regs16[seg_override_en ? seg_override : bios_table_lookup[scratch2_uint + 3][i_rm]] + (unsigned short)(regs16[bios_table_lookup[scratch2_uint + 1][i_rm]] + bios_table_lookup[scratch2_uint + 2][i_rm] * i_data1+ regs16[bios_table_lookup[scratch2_uint][i_rm]]) : (REGS_BASE + (i_w ? 2 * i_rm : 2 * i_rm + i_rm / 4 & 7)), op_from_addr = (REGS_BASE + (i_w ? 2 * i_reg : 2 * i_reg + i_reg / 4 & 7)), i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint);
                                scratch2_uint = 4 * !i_mod, op_to_addr = rm_addr = i_mod < 3 ? 16L * read_regs16(seg_override_en ? seg_override : read_bios_table_lookup(scratch2_uint + 3,i_rm)) + (unsigned short)(read_regs16(read_bios_table_lookup(scratch2_uint + 1,i_rm)) + read_bios_table_lookup(scratch2_uint + 2,i_rm) * i_data1+ read_regs16(read_bios_table_lookup(scratch2_uint,i_rm))) : (REGS_BASE + ( 2 * i_rm )), op_from_addr = (REGS_BASE + ( 2 * i_reg )), i_d && (scratch_uint = op_from_addr, op_from_addr = rm_addr, op_to_addr = scratch_uint);
				//(i_w ? op_dest = *(unsigned short*)&mem[op_to_addr], op_result = *(unsigned short*)&mem[op_to_addr] = (op_source = *(unsigned short*)&mem[op_from_addr]) : (op_dest = mem[op_to_addr], op_result = mem[op_to_addr] = (op_source = *(unsigned char*)&mem[op_from_addr])));
                                (op_dest = read_ram16(op_to_addr), write_ram16( op_to_addr, op_source = read_ram16(op_from_addr)), op_result = read_ram16( op_to_addr ) );
				//(i_w ? op_dest = *(unsigned short*)&mem[REGS_BASE + extra], op_result = *(unsigned short*)&mem[REGS_BASE + extra]  = (op_source = *(unsigned short*)&mem[ rm_addr + 2]) : (op_dest = mem[REGS_BASE + extra], op_result = mem[REGS_BASE + extra]  = (op_source = *(unsigned char*)&mem[ rm_addr + 2])))
                                (op_dest = read_ram16(REGS_BASE + extra), write_ram16(REGS_BASE + extra, op_source = read_ram16( rm_addr + 2)), op_result = read_ram16( REGS_BASE + extra) )
			;break; case 38: // INT 3
				++reg_ip;
				pc_interrupt(3)
			;break; case 39: // INT imm8
				reg_ip += 2;
				pc_interrupt(i_data0)
			;break; case 40: // INTO
				++reg_ip;
				//regs8[FLAG_OF] && pc_interrupt(4)
                                read_regs8(FLAG_OF) && pc_interrupt(4)
			;break; case 41: // AAM
				if (i_data0 &= 0xFF)
					//regs8[REG_AH] = regs8[REG_AL] / i_data0,
                                        write_regs8(REG_AH, read_regs8(REG_AL) / i_data0),
					//op_result = regs8[REG_AL] %= i_data0;
                                        write_regs8(REG_AL, read_regs8(REG_AL) % i_data0), op_result = read_regs8(REG_AL);
				else // Divide by zero
					pc_interrupt(0)
			;break; case 42: // AAD
				i_w = 0;
				//regs16[REG_AX] = op_result = 0xFF & regs8[REG_AL] + i_data0 * regs8[REG_AH]
                                write_regs16(REG_AX, op_result = 0xFF & read_regs8(REG_AL) + i_data0 * read_regs8(REG_AH))
			;break; case 43: // SALC
				//regs8[REG_AL] = -regs8[FLAG_CF]
                                write_regs8(REG_AL, -read_regs8(FLAG_CF))
			;break; case 44: // XLAT
				//regs8[REG_AL] = mem[16 * regs16[seg_override_en ? seg_override : REG_DS] + (unsigned short)( regs8[REG_AL] + regs16[ REG_BX])]
                                write_regs8(REG_AL, read_ram8(16L * read_regs16(seg_override_en ? seg_override : REG_DS) + (unsigned short)( read_regs8(REG_AL)+ read_regs16( REG_BX))))
			;break; case 45: // CMC
				//regs8[FLAG_CF] ^= 1
                                write_regs8(FLAG_CF, read_regs8(FLAG_CF) ^ 1)
			;break; case 46: // CLC|STC|CLI|STI|CLD|STD
				//regs8[extra / 2] = extra & 1
                                write_regs8(extra / 2, extra & 1)
			;break; case 47: // TEST AL/AX, immed
				//(i_w ? op_dest = *(unsigned short*)&regs8[REG_AL], op_result = *(unsigned short*)&regs8[REG_AL]  & (op_source = *(unsigned short*)& i_data0) : (op_dest = regs8[REG_AL], op_result = regs8[REG_AL]  & (op_source = *(unsigned char*)& i_data0)))
                                (i_w ? op_dest = read_regs16(REG_AL), op_result = read_regs16(REG_AL)  & (op_source = *(unsigned short*)& i_data0) : (op_dest = read_regs8(REG_AL), op_result = read_regs8(REG_AL)  & (op_source = *(unsigned char*)& i_data0)))
			;break; case 48: // Emulator-specific 0F xx opcodes
				//switch ((char)i_data0)
                                switch ((int8_t)i_data0)
				{
					; case 0: // PUTCHAR_AL
						//write(1, regs8, 1)
                                                //putchar( read_regs8(REG_AL) )
                                                { uint8_t buffer = read_regs8(REG_AL); write_console( &buffer ); }
					;break; case 1: // GET_RTC
#if 0
						time(&clock_buf);
						ftime(&ms_clock);
						//memcpy(mem + 16 * regs16[REG_ES] + (unsigned short)( regs16[ REG_BX]), localtime(&clock_buf), sizeof(struct tm));
                                                {
                                                char* tmptr = (char*)localtime(&clock_buf);
                                                for( int i = 0; i < sizeof(struct tm); i++ ) {
                                                  write_ram8( 16L*read_regs16(REG_ES) + read_regs16(REG_BX) + i, tmptr[ i ] );
                                                }
                                                }
						//*(short*)&mem[16 * regs16[REG_ES] + (unsigned short)( 36+ regs16[ REG_BX])] = ms_clock.millitm;
                                                write_ram16(16L * read_regs16(REG_ES) + (unsigned short)( 36+ read_regs16( REG_BX)), ms_clock.millitm );
#else
                                                getrtc();
#endif
					;break; case 2: // DISK_READ
					; case 3: // DISK_WRITE
#if 0
						regs8[REG_AL] = ~lseek(disk[regs8[REG_DL]], *(unsigned*)&regs16[REG_BP] << 9, 0)
							? ((char)i_data0 == 3 ? (int(*)())write : (int(*)())read)(disk[regs8[REG_DL]], mem + 16 * regs16[REG_ES] + (unsigned short)( regs16[ REG_BX]), regs16[REG_AX])
							: 0;
#else
                                                if( i_data0 == 3 ) {
                                                  // write()
                                                  for( int i = 0; i < read_regs16(REG_AX); i++ ) {
                                                    write_disk( read_regs8(REG_DL),  ((uint32_t)read_regs16(REG_BP)<<9) + i, read_ram8( 16L * read_regs16(REG_ES) + read_regs16(REG_BX) + i) );
                                                  }
                                                } else {
                                                  // read()
                                                  for( int i = 0; i < read_regs16(REG_AX); i++ ) {
                                                    write_ram8( 16L * read_regs16(REG_ES) + read_regs16(REG_BX) + i, read_disk( read_regs8(REG_DL),  ((uint32_t)read_regs16(REG_BP)<<9) + i ) );
                                                  }
                                                }
#endif
				}
		}

		// Increment instruction pointer by computed instruction length. Tables in the BIOS binary
		// help us here.
		//reg_ip += (i_mod*(i_mod != 3) + 2*(!i_mod && i_rm == 6))*i_mod_size + bios_table_lookup[TABLE_BASE_INST_SIZE][raw_opcode_id] + bios_table_lookup[TABLE_I_W_SIZE][raw_opcode_id]*(i_w + 1);
                reg_ip += (i_mod*(i_mod != 3) + 2*(!i_mod && i_rm == 6))*i_mod_size + read_bios_table_lookup(TABLE_BASE_INST_SIZE,raw_opcode_id) + read_bios_table_lookup(TABLE_I_W_SIZE,raw_opcode_id)*(i_w + 1);

		// If instruction needs to update SF, ZF and PF, set them as appropriate
		if (set_flags_type & FLAGS_UPDATE_SZP)
		{
			//regs8[FLAG_SF] = (1 & (i_w ? *(short*)&op_result : op_result) >> (8*(i_w + 1) - 1));
                        write_regs8(FLAG_SF, 1 & (i_w ? *(short*)&op_result : op_result) >> (8*(i_w + 1) - 1));
			//regs8[FLAG_ZF] = !op_result;
                        write_regs8(FLAG_ZF, !op_result);
			//regs8[FLAG_PF] = bios_table_lookup[TABLE_PARITY_FLAG][(unsigned char)op_result];
                        write_regs8(FLAG_PF, read_bios_table_lookup(TABLE_PARITY_FLAG,(unsigned char)op_result));

			// If instruction is an arithmetic or logic operation, also set AF/OF/CF as appropriate.
			if (set_flags_type & FLAGS_UPDATE_AO_ARITH)
				set_AF_OF_arith();
			if (set_flags_type & FLAGS_UPDATE_OC_LOGIC)
				set_CF(0), set_OF(0);
		}

		// Poll timer/keyboard every KEYBOARD_TIMER_UPDATE_DELAY instructions
		if (!(++inst_counter % KEYBOARD_TIMER_UPDATE_DELAY))
			int8_asap = 1;

#if 0
		// Update the video graphics display every GRAPHICS_UPDATE_DELAY instructions
		if (!(inst_counter % GRAPHICS_UPDATE_DELAY))
		{
			// Video card in graphics mode?
			if (io_ports[0x3B8] & 2)
			{
				// If we don't already have an SDL window open, set it up and compute color and video memory translation tables
				if (!sdl_screen)
				{
					for (int i = 0; i < 16; i++)
						pixel_colors[i] = mem[0x4AC] ? // CGA?
							cga_colors[(i & 12) >> 2] + (cga_colors[i & 3] << 16) // CGA -> RGB332
							: 0xFF*(((i & 1) << 24) + ((i & 2) << 15) + ((i & 4) << 6) + ((i & 8) >> 3)); // Hercules -> RGB332

					for (int i = 0; i < GRAPHICS_X * GRAPHICS_Y / 4; i++)
						vid_addr_lookup[i] = i / GRAPHICS_X * (GRAPHICS_X / 8) + (i / 2) % (GRAPHICS_X / 8) + 0x2000*(mem[0x4AC] ? (2 * i / GRAPHICS_X) % 2 : (4 * i / GRAPHICS_X) % 4);

					SDL_Init(SDL_INIT_VIDEO);
					sdl_screen = SDL_SetVideoMode(GRAPHICS_X, GRAPHICS_Y, 8, 0);
					SDL_EnableUNICODE(1);
					SDL_EnableKeyRepeat(500, 30);
				}

				// Refresh SDL display from emulated graphics card video RAM
				vid_mem_base = mem + 0xB0000 + 0x8000*(mem[0x4AC] ? 1 : io_ports[0x3B8] >> 7); // B800:0 for CGA/Hercules bank 2, B000:0 for Hercules bank 1
				for (int i = 0; i < GRAPHICS_X * GRAPHICS_Y / 4; i++)
					((unsigned *)sdl_screen->pixels)[i] = pixel_colors[15 & (vid_mem_base[vid_addr_lookup[i]] >> 4*!(i & 1))];

				SDL_Flip(sdl_screen);
			}
			else if (sdl_screen) // Application has gone back to text mode, so close the SDL window
			{
				SDL_QuitSubSystem(SDL_INIT_VIDEO);
				sdl_screen = 0;
			}
			SDL_PumpEvents();
		}
#endif

		// Application has set trap flag, so fire INT 1
		if (trap_flag)
			pc_interrupt(1);

		//trap_flag = regs8[FLAG_TF];
                trap_flag = read_regs8(FLAG_TF);

		// If a timer tick is pending, interrupts are enabled, and no overrides/REP are active,
		// then process the tick and check for new keystrokes
		//if (int8_asap && !seg_override_en && !rep_override_en && regs8[FLAG_IF] && !regs8[FLAG_TF])
                if (int8_asap && !seg_override_en && !rep_override_en && read_regs8(FLAG_IF) && !read_regs8(FLAG_TF)) {
			//pc_interrupt(0xA), int8_asap = 0, read(0, mem + 0x4A6, 1) && (int8_asap = (mem[0x4A6] == 0x1B), pc_interrupt(7));
                        //pc_interrupt(0xA), int8_asap = 0, write_ram8( 0x4A6, getchar()), 1 && (int8_asap = read_ram8( 0x4A6 ) == 0x1B, pc_interrupt(7));
                        uint8_t temp;
                        pc_interrupt(0xA), int8_asap = 0, read_console(&temp) && (write_ram8( 0x4A6, temp ), 1) && (int8_asap = read_ram8( 0x4A6 ) == 0x1B, pc_interrupt(7));
                }
	}

#if 0
	SDL_Quit();
#endif
	return 0;
}

