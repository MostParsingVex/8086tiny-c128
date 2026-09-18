#include<unistd.h>

char buffer[ 512 ] = "\xb8\x48\x0e" // mov ax, 0xe48
                     "\xcd\x10"     // int 0x10
                     "\xb0\x65"     // mov al, 0x65
                     "\xcd\x10"     // int 0x10
                     "\xb0\x6c"     // mov al, 0x6c
                     "\xcd\x10"     // int 0x10
                     "\xb0\x6c"     // mov al, 0x6c
                     "\xcd\x10"     // int 0x10
                     "\xb0\x6f"     // mov al, 0x6f
                     "\xcd\x10"     // int 0x10
                     "\xb0\x2c"     // mov al, 0x2c
                     "\xcd\x10"     // int 0x10
                     "\xb0\x20"     // mov al, 0x20
                     "\xcd\x10"     // int 0x10
                     "\xb0\x57"     // mov al, 0x57
                     "\xcd\x10"     // int 0x10
                     "\xb0\x6f"     // mov al, 0x6f
                     "\xcd\x10"     // int 0x10
                     "\xb0\x72"     // mov al, 0x72
                     "\xcd\x10"     // int 0x10
                     "\xb0\x6c"     // mov al, 0x6c
                     "\xcd\x10"     // int 0x10
                     "\xb0\x64"     // mov al, 0x64
                     "\xcd\x10"     // int 0x10
                     "\xb0\x21"     // mov al, 0x21
                     "\xcd\x10"     // int 0x10
                     "\xeb\xfe"     // l: jmp l
;
int main() {
  buffer[510] = 0x55;
  buffer[511] = 0xaa;
  write( 1, buffer, 512 );
}
