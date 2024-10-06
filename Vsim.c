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
int x0 = 0;
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
  int imm1; // imm is just 1 but it's split
  int rs1;
  int rs2;
  // int func3
  // int imm2;
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

    // create copy of line without line endings
    char* cpy = malloc(33 * sizeof(char));
    strncpy(cpy, line, 32);
    cpy[32] = '\0';

    // create queue item
    entry* item = malloc(sizeof(entry));
    item->line  = cpy;

    // for reading signed integers after break
    if (endFlag == true) {
      // make sure a new memory location is made for the int
      int* num = malloc(sizeof(int));
      *num     = (int) strtol(line, NULL, 2);

      // put the int on the memory queue
      STAILQ_INSERT_TAIL(&memqueue, item, next);
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
    // printf("cat: %d, opcode: %d ", category + 1, opcode);
    opcodes[category][opcode]();

    // get arguments based on category

    char* im;
    int imm = 0;

    // rd is dest, rs1 and rs2 are source, all located in same places
    char reg[6];
    int rd = 0;
    strncpy(reg, &line[20], 5);
    reg[5] = '\0';
    rd     = (int) strtol(reg, NULL, 2);

    // extract rs1 from text
    char r1[6];
    int rs1 = 0;
    strncpy(r1, &line[12], 5);
    r1[5] = '\0';
    rs1   = (int) strtol(r1, NULL, 2);

    // exctract rs2 from text
    char r2[6];
    strncpy(r2, &line[7], 5);
    r2[5]   = '\0';
    int rs2 = (int) strtol(r2, NULL, 2);

    // create instructions for each category and place in queue
    switch (category) {
    // cat 4
    case 0:
      // Extract imm from text
      im = malloc(20 * sizeof(char));
      strncpy(im, line, 19);
      im[19] = '\0';
      // convert to decimal
      imm    = (int) strtol(im, NULL, 2);

      // create instruction and insert into item
      cat_4* instruction4    = malloc(sizeof(cat_4));
      instruction4->category = category;
      instruction4->opcode   = opcode;
      instruction4->imm1     = imm;
      instruction4->rd       = rd;
      item->data             = (void*) instruction4;

      // place item on queue
      STAILQ_INSERT_TAIL(&memqueue, item, next);
      break;
    // cat 2
    case 1:

      // create instruction and insert into item
      cat_2* instruction2    = malloc(sizeof(cat_2));
      instruction2->category = category;
      instruction2->opcode   = opcode;
      instruction2->rd       = rd;
      instruction2->rs1      = rs1;
      instruction2->rs2      = rs2;
      item->data             = instruction2;

      // place item on queue
      STAILQ_INSERT_TAIL(&memqueue, item, next);
      break;
    // cat 3
    case 2:
      // extract imm from text
      im = malloc(12 * sizeof(char));
      strncpy(im, line, 11);
      im[11] = '\0';
      // convert to decimal
      imm    = (int) strtol(im, NULL, 2);

      // create instruction and insert into item
      cat_3* instruction3    = malloc(sizeof(cat_3));
      instruction3->category = category;
      instruction3->opcode   = opcode;
      instruction3->imm1     = imm;
      instruction3->rs1      = rs1;
      instruction3->rd       = rd;
      item->data             = instruction3;

      // place item on queue
      STAILQ_INSERT_TAIL(&memqueue, item, next);
      break;
    // cat 1
    case 3:
      // extract imm1 and imm2 from text
      im = malloc(13 * sizeof(char));
      strncpy(im, line, 7);
      im[8] = '\0';
      // rd location has the rest of the integer
      strcpy(im, reg);
      imm = (int) strtol(im, NULL, 2);

      cat_1* instruction1    = malloc(sizeof(cat_1));
      instruction1->category = category;
      instruction1->opcode   = opcode;
      instruction1->imm1     = imm;
      instruction1->rs1      = rs1;
      instruction1->rs2      = rs2;

      // place item on queue
      STAILQ_INSERT_TAIL(&memqueue, item, next);
      break;
    }

    // printf("%s\n", op);
  }
}