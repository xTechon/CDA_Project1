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
  // int func3
  int imm2;
} cat_1;

typedef struct cat_2
{
  int category;
  int opcode;
  int rd;
  // int func3;
  int rs1;
  int rs2;
  // int func7;
} cat_2;

typedef struct cat_3
{
  int category;
  int opcode;
  int rd;
  // func3
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

typedef struct entry
{
  block data;               // store the instruction
  char* line;               // store the original binary
  STAILQ_ENTRY(entry) next; // link to next portion of memory
} entry;

STAILQ_HEAD(stailhead, entry); // create the type for head of the queue

struct stailhead memqueue; // queue to push parsed instructions

void parseFile(FILE* fp) {

  // Init the file reader
  char line[35]; // 32 bit word + \n\r + \0
  char op[6];

  // flag for the end of a program
  bool endFlag = false;

  // init the queue
  STAILQ_INIT(&memqueue);

  // Itterate over every line in the file
  while (fgets(line, 35, fp) != NULL) {

    // for reading signed integers after break
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

    // create copy of line
    char cpy[33];
    strncpy(cpy, line, 32);
    cpy[32] = '\0';

    // create instructions for each category and place in queue
    switch (category) {
    // cat 4
    case 0:
      // Extract imm from text
      char im[20];
      strncpy(im, line, 19);
      im[19]  = '\0';
      // convert to decimal
      int imm = (int) strtol(im, NULL, 2);

      // Extract rd from text (len 6 @ 20)
      char reg[7];
      strncpy(reg, &line[20], 6);
      reg[6] = '\0';
      int rd = (int) strtol(reg, NULL, 2);

      // create instruction
      cat_4 instruction = {.category = category, .opcode = opcode, .imm1 = imm, .rd = rd};
      // create queue item
      entry* item       = malloc(sizeof(entry));
      item->data        = &instruction;
      item->line        = cpy;

      // place item on queue
      STAILQ_INSERT_HEAD(&memqueue, item, next);
      break;
    // cat 2
    case 1:
      // instruction = malloc(sizeof(cat_2));
      break;
    // cat 3
    case 2:
      // instruction = malloc(sizeof(cat_3));
      break;
    // cat 1
    case 3:
      // instruction = malloc(sizeof(cat_1));
      break;
    }

    // printf("%s\n", op);
  }
}