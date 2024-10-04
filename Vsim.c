// On my honor, I have neither given nor recieved ay unauthroized aid on this assignment
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

// Parse the input file
void parseFile(FILE* fp);

// argc is # of arguments including program execution
// argv is the array of strings of every argument including execution
int main(int argc, char* argv[]) {

  if (argc != 2) { // check to see if correct amnt of arguments entered
    printf("ERROR: Usage: %s filename\n", argv[0]);
    return 0;
  }

  // --- Load text file ---
  FILE* fp;
  fp = fopen(argv[1], "r");

  // --- Error handle file ---
  if (fp == NULL) {
    printf("ERROR: %s could not be opened for reading.\n", argv[1]);
    return 0;
  }

  // --- Parse file and load into "memory" ---
  parseFile(fp);

  fclose(fp);
  return 0;
}

// Define type to point to functions
typedef void (*func_type)(void);

// Define Registers
int x0 = 10;
int x1, x2, x3, x4, x5, x6, x7, x8         = 0;
int x9, x10, x11, x12, x13, x14, x15, x16  = 0;
int x17, x18, x19, x20, x21, x22, x23, x24 = 0;
int x25, x26, x27, x28, x29, x30, x31 = 0;

// Make registers callable dynamically
int* registers[32] = {&x0,  &x1,  &x2,  &x3,  &x4,  &x5,  &x6,  &x7,  &x8,  &x9,  &x10, &x11, &x12, &x13, &x14, &x15,
                      &x16, &x17, &x18, &x19, &x20, &x21, &x22, &x23, &x24, &x25, &x26, &x27, &x28, &x29, &x30, &x31};

const int offset = 256;
int pc           = 0; // assignment defines input as starting at address 256

// Category 1, S-Type instructions
void beq() { puts("beq"); }

void bne() { puts("bne"); };

void blt() { puts("blt"); };

void sw() { puts("sw"); };

// Category 2, R-Type instructions
void add() { puts("add"); }

void sub() { puts("sub"); }

void and () { puts("and "); }

void or () { puts("or "); }

// Category 3, I-Type instructions
void addi() { puts("addi"); }

void andi() { puts("andi"); }

void ori() { puts("ori"); }

void sll() { puts("sll"); }

void sra() { puts("sra"); }

void lw() { puts("lw"); }

// Category 4, U-Type instructions
void jal() { puts("jal"); }

void br() { puts("break"); } // Can't use "break" as it's a C keyword

// Opcode mapping table
func_type c1[4]       = {beq, bne, blt, sw};
func_type c2[4]       = {add, sub, and, or };
func_type c3[6]       = {addi, andi, ori, sll, sra, lw};
func_type c4[2]       = {jal, br};
func_type* opcodes[4] = {c4, c2, c3, c1};

// define instruction types
typedef struct cat_1
{
  int category;
  int opcode;
  int imm1;
  int rs1;
  int rs2;
  int imm2;
} cat_1;

typedef struct cat_2
{
  int category;
  int opcode;
  int rd;
  int rs1;
  int rs2;
  int func7;
} cat_2;

typedef struct cat_3
{
  int category;
  int opcode;
  int rd;
  int rs1;
  int imm1;
} cat_3;

typedef struct cat_4
{
  int category;
  int opcode;
  int rd;
  int imm1;
} cat_4;

// create a queue to place lines of the file
typedef void* block;

struct entry
{
  block data;               // store the instruction
  char* line;               // store the original binary
  STAILQ_ENTRY(entry) next; // link to next portion of memory
};

STAILQ_HEAD(stailhead, entry);

struct stailhead MEMORY;

void parseFile(FILE* fp) {

  // Init the file reader
  char line[35]; // 32 bit word + \n\r + \0
  char op[6];

  bool endFlag = false;

  // Itterate over every line in the file
  while (fgets(line, 35, fp) != NULL) {

    if (endFlag == true) {
      int num = (int) strtol(line, NULL, 2);
      printf("number = %d\n", num);
      continue;
    }

    // Get the category code
    char catStr[3] = {line[30], line[31], '\0'};
    short category = (short) strtol(catStr, NULL, 2);

    // Process opcodes
    strncpy(op, &line[25], 5);              // Get the opcode
    op[5]      = '\0';                      // Terminate Manually
    int opcode = (int) strtol(op, NULL, 2); // convert to decimal

    // 11111, start reading integer values
    if (opcode == 31) {
      opcode  = 1;
      endFlag = true;
    }
    printf("cat: %d, opcode: %d ", category + 1, opcode);
    opcodes[category][opcode]();

    // get arguments based on category

    void* instruction;
    // func_type* opcodes[4] = {c4, c2, c3, c1};
    switch (category) {
    // cat 4
    case 0:
      instruction = malloc(sizeof(cat_4));
      break;
    // cat 2
    case 1:
      instruction = malloc(sizeof(cat_2));
      break;
    // cat 3
    case 2:
      instruction = malloc(sizeof(cat_3));
      break;
    // cat 1
    case 3:
      instruction = malloc(sizeof(cat_1));
      break;
    }

    // printf("%s\n", op);
  }
}