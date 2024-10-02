// On my honor, I have neither given nor recieved ay unauthroized aid on this assignment

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// argc is # of arguments including program execution
// argv is the array of strings of every execution
int main(int argc, char* argv[]) {

  if (argc != 2) { // check to see if correct amnt of arguments entered
    printf("ERROR: Usage: %s filename\n", argv[0]);
    return 0;
  }

  // --- Load text file ---
  FILE* fp;
  fp = fopen(argv[1], "r");

  // Error handle file
  if (fp == NULL) {
    printf("ERROR: %s could not be opened for reading.\n", argv[1]);
    return 0;
  }

  int intType;
  printf("Size of int: %zu bytes\n", sizeof(intType));

  // Init the file reader
  char line[35]; // 32 bit word + \n\r + \0

  // Itterate over every line in the file
  while (fgets(line, 1024, fp) != NULL) {
    printf("%s", line);
  }

  fclose(fp);
  return 0;
}

// Define Registers
const int x0 = 0;
int x1, x2, x3, x4, x5, x6, x7, x8         = 0;
int x9, x10, x11, x12, x13, x14, x15, x16  = 0;
int x17, x18, x19, x20, x21, x22, x23, x24 = 0;
int x25, x26, x27, x28, x29, x30, x31 = 0;
int pc = 256; // assignment defines input as starting at address 256

// Category 1, S-Type instructions
void beq();
void bne();
void blt();
void sw();

// Category 2, R-Type instructions
void add();
void sub();
void and ();
void or ();

// Category 3, I-Type instructions
void addi();
void andi();
void ori();
void ssl();
void sra();
void lw();

// Category 4, U-Type instructions
void jal();
void br(); // Can't use "break" as it's a C keyword