#include<stdio.h>
#include"reu.h"

int main(){
  ram_init();

  puts("clear ram");
  for( uint32_t i = 0; i < 0x120000L; i++ ) {
    write_ram8( i, 0 );
  }

  puts("loading bios");
  FILE *fp = fopen( "bios", "rb" );
  uint8_t val;
  for( int i = 0; i < 8192; i++ ) {
    fread( &val, 1, 1, fp );
    write_ram8( 0xf0100L + i, val );
  }  
  fclose( fp );
#if 1
  puts("loading fd.img");
  fp = fopen( "fd.img", "rb" );
  for( uint32_t i = 0; i < 1474560; i++ ) {
    if( (i & 0x3ff) == 0 ) printf( "%ld\n", (long)i );
    fread( &val, 1, 1, fp );
    write_disk( 0, i, val );
  }
#endif

#ifdef C128
  asm("jmp $ff3d");
#else
  asm("jmp $fce2");
#endif
}
