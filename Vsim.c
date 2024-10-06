// On my honor, I have neither given nor recieved ay unauthroized aid on this assignment
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

// Parse the input file
void parseFile(FILE* fp);

// load queue into array for executing
// prints program contents to file as well
void loadQueueToMemory();

// create a queue to place lines of the file
typedef void* block;

typedef struct entry
{
  block data;               // store the instruction
  short category;           // add information about category and opcode
  int opcode;               // avoids the need for type reflection (not possible in C to my knowledge)
  char* line;               // store the original binary
  STAILQ_ENTRY(entry) next; // link to next portion of memory
} entry;

STAILQ_HEAD(stailhead, entry); // create the type for head of the queue

struct stailhead memqueue; // queue to push parsed instructions

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

  // --- Parse file and load into "memory" queue ---
  parseFile(fp);

  // --- move the queue into an array for execution --
  // --- also generates the string to print assembly to file --
  loadQueueToMemory();

  fclose(fp);
  return 0;
}

// Define type to point to functions
// Useing void * as polymorphism to allow for function table later
typedef char* (*func_type)(void*);

// define instruction types
typedef struct cat_1
{
  int imm1; // imm is just 1 but it's split
  int rs1;
  int rs2;
} cat_1;

typedef struct cat_2
{
  int rd;
  int rs1;
  int rs2;
} cat_2;

typedef struct cat_3
{
  int rd;
  int rs1;
  int imm1;
} cat_3;

typedef struct cat_4
{
  int rd;
  int imm1;
} cat_4;

// Define Registers
int registers[32];

// assignment defines input as starting at address 256
const int offset = 256;
int pc           = 0;

bool exec = false; // toggle function execution

// generates assembly string from cat1 instructions
char* cat1String(char* instr, cat_1* instruction) {
  char* assem = malloc(20 * sizeof(char));
  sprintf(assem, "%s x%d, x%d, #%d", instr, instruction->rs1, instruction->rs2, instruction->imm1);
  return assem;
}

// Category 1, S-Type instructions
char* beq(void* instruction) {
  // generate assembly string
  cat_1* instr = (cat_1*) instruction;
  char* assem  = cat1String("beq", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

char* bne(void* instruction) {
  cat_1* instr = (cat_1*) instruction;
  char* assem  = cat1String("bne", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

char* blt(void* instruction) {
  cat_1* instr = (cat_1*) instruction;
  char* assem  = cat1String("blt", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

// BUG: not reading correct imm number
char* sw(void* instruction) {
  cat_1* instr = (cat_1*) instruction;
  char* assem  = malloc(20 * sizeof(char));
  sprintf(assem, "sw x%d, %d(x%d)", instr->rs1, instr->imm1, instr->rs2);
  if (exec == false) { return assem; }
  return assem;
}

// generates assembly string from cat1 instructions
char* cat2String(char* instr, cat_2* instruction) {
  char* assem = malloc(20 * sizeof(char));
  sprintf(assem, "%s x%d, x%d, x%d", instr, instruction->rd, instruction->rs1, instruction->rs2);
  return assem;
}

// Category 2, R-Type instructions
char* add(void* instruction) {
  cat_2* instr = (cat_2*) instruction;
  char* assem  = cat2String("add", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

char* sub(void* instruction) {
  cat_2* instr = (cat_2*) instruction;
  char* assem  = cat2String("sub", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

char* and (void* instruction) {
  cat_2* instr = (cat_2*) instruction;
  char* assem  = cat2String("and", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

char* or (void* instruction) {
  cat_2* instr = (cat_2*) instruction;
  char* assem  = cat2String("and", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

// generates assembly string from cat1 instructions
char* cat3String(char* instr, cat_3* instruction) {
  char* assem = malloc(20 * sizeof(char));
  sprintf(assem, "%s x%d, x%d, #%d", instr, instruction->rd, instruction->rs1, instruction->imm1);
  return assem;
}

// Category 3, I-Type instructions
char* addi(void* instruction) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = cat3String("addi", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

char* andi(void* instruction) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = cat3String("andi", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

char* ori(void* instruction) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = cat3String("ori", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

char* sll(void* instruction) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = cat3String("sll", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

char* sra(void* instruction) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = cat3String("sra", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

// BUG: not reading correct immediate number
char* lw(void* instruction) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = malloc(20 * sizeof(char));
  sprintf(assem, "lw x%d, %d(x%d)", instr->rd, instr->imm1, instr->rs1);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

bool ENDFLAG = false;

// Category 4, U-Type instructions
// BUG: imm not read as 2's complement
// Results in not reading negative values
char* jal(void* instruction) {
  cat_4* instr = (cat_4*) instruction;
  char* assem  = malloc(20 * sizeof(char));
  sprintf(assem, "jal x%d, #%d", instr->rd, instr->imm1);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  return assem;
}

// will set global program flag to true
// Prevents parsing further code if ran
char* br(void* instruction) {
  ENDFLAG = true;
  return "break";
} // Can't use "break" as it's a C keyword

// Opcode mapping table
func_type c1[4]       = {beq, bne, blt, sw};
func_type c2[4]       = {add, sub, and, or };
func_type c3[6]       = {addi, andi, ori, sll, sra, lw};
func_type c4[2]       = {jal, br};
func_type* opcodes[4] = {c4, c2, c3, c1};

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
      int* num   = malloc(sizeof(int));
      *num       = (int) strtol(line, NULL, 2);
      item->data = num;

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

    // add category and opcode to queue item
    item->category = category;
    item->opcode   = opcode;

    // get arguments based on category
    char* im;
    int imm = 0;

    // rd is dest, rs1 and rs2 are source, all located in same places
    // extract rd from text
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
      imm = (int) strtol(im, NULL, 2);

      // create instruction and insert into item
      cat_4* instruction4 = malloc(sizeof(cat_4));
      instruction4->imm1  = imm;
      instruction4->rd    = rd;

      // add instruction ptr to queue item
      item->data = instruction4;

      // place item on queue
      STAILQ_INSERT_TAIL(&memqueue, item, next);
      break;
    // cat 2
    case 1:
      // create instruction and insert into item
      cat_2* instruction2 = malloc(sizeof(cat_2));
      instruction2->rd    = rd;
      instruction2->rs1   = rs1;
      instruction2->rs2   = rs2;

      // add instruction ptr to queue item
      item->data = instruction2;

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
      imm = (int) strtol(im, NULL, 2);

      // create instruction and insert into item
      cat_3* instruction3 = malloc(sizeof(cat_3));
      instruction3->imm1  = imm;
      instruction3->rs1   = rs1;
      instruction3->rd    = rd;

      // add instruction ptr to queue item
      item->data = instruction3;

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

      // create instruction and insert into item
      cat_1* instruction1 = malloc(sizeof(cat_1));
      instruction1->imm1  = imm;
      instruction1->rs1   = rs1;
      instruction1->rs2   = rs2;

      // add instruction ptr to queue item
      item->data = instruction1;

      // place item on queue
      STAILQ_INSERT_TAIL(&memqueue, item, next);
      break;
    }
  }
}

// parse over the entire array
void loadQueueToMemory() {
  entry* item;
  // triggers end of program
  STAILQ_FOREACH(item, &memqueue, next) {
    if (ENDFLAG == true) {
      printf("%d\n", *((int*) item->data));
      continue;
    }
    char* assem = opcodes[item->category][item->opcode](item->data);
    printf("%s\n", assem);
  }
  // reset for next run
  ENDFLAG = false;
}