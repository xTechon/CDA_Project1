// On my honor, I have neither given nor recieved any unauthroized aid on this assignment
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>

// Parse the input file
void parseFile(FILE* fp);

// load queue into array for executing
// prints program contents to file as well
char* loadQueueToMemory();

// runs the program and returns cycles for simulation output
void executeProgram();

// prints the output of program execution to a file
void printToFile();

// create a queue to place lines of the file
typedef void* block;

// define item for queues
typedef struct entry
{
  block data;               // store the instruction
  short category;           // add information about category and opcode
  int opcode;               // avoids the need for type reflection (not possible in C to my knowledge)
  char* line;               // store the original binary
  char* assem;              // store the assembly of the instruction
  int counter;              // store the state of PC at time of fetch
  bool moved;               // flag to check if instruction has been moved in current cycle
  STAILQ_ENTRY(entry) next; // link to next portion of memory
} entry;

STAILQ_HEAD(stailhead, entry); // create the type for head of the queue

struct stailhead memqueue;      // queue to push parsed instructions
struct stailhead cycleQueue;    // queue to hold cycles on
struct stailhead preIssueQueue; // queue for pre-issues
struct stailhead preALU1Queue;  // queue for ALU1
struct stailhead preALU2Queue;  // queue for ALU2
entry* IFUnitWait     = NULL;   // wait on the IF unit
entry* IFUnitExecuted = NULL;   // execute on the IF unit
entry* preMEMQueue    = NULL;   // 1 Entry Queue for pre-MEM
entry* postMEMQueue   = NULL;   // 1 Entry Queue for post-MEM
entry* postALU2Queue  = NULL;   // 1 Entry Queue for post-ALU2
int programSize       = 0;      // size of program in lines/words

// argc is # of arguments including program execution
// argv is the array of strings of every argument including execution
int main(int argc, char* argv[]) {

  // check to see if correct amnt of arguments entered
  if (argc != 2) {
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
  char* disassembly = loadQueueToMemory();

  // --- Create/Overwrite file with disassembly
  FILE* disAssm = fopen("disassembly.txt", "w");
  fprintf(disAssm, "%s", disassembly);
  fclose(disAssm);

  // --- Run the program ---
  executeProgram();

  // --- Print the output to a file ---
  printToFile();

  fclose(fp);
  return 0;
}

// Define type to point to functions
// Useing void * as polymorphism to allow for function table later
typedef char* (*func_type)(void*, int);

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
int cycle        = 1;

// initalize program array memory
// points to existing objects in queue
entry** memory;
// points to where data starts in the memory array
entry** data  = NULL;
int dCounter  = 0; // size of data
int dataStart = 0; // start address of data

bool exec    = false; // toggle function execution
bool ENDFLAG = false; // marks end of program/start of literals

// track sizes of different queues
int preIssueQueueSize           = 0;
int preALU1QueueSize            = 0;
int preALU2QueueSize            = 0;
int numALU2instr                = 0; // number of ALU2 instr in pipeline
// Const limits on queue sizes
const int PRE_ISSUE_QUEUE_LIMIT = 4;
const int PRE_ALU1_QUEUE_LIMIT  = 2;
const int PRE_ALU2_QUEUE_LIMIT  = 2;

// Fetch unit flag
bool IF_HALT = false;

// generates assembly string from cat1 instructions
char* cat1String(char* instr, cat_1* instruction) {
  char* assem = malloc(20 * sizeof(char));
  sprintf(assem, "%s x%d, x%d, #%d", instr, instruction->rs1, instruction->rs2, instruction->imm1);
  return assem;
}

// Category 1, S-Type instructions
char* beq(void* instruction, int counter) {
  // generate assembly string
  cat_1* instr = (cat_1*) instruction;
  char* assem  = cat1String("beq", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // check if rs1 == rs2, if fails, exit function
  if (registers[instr->rs1] != registers[instr->rs2]) { return assem; }
  // get the current address
  int cur    = (counter * 4) + offset;
  // left shift immediate
  int shift  = instr->imm1 * 2;
  // create target address
  int target = cur + shift;
  // convert target address to index
  target     = (target - offset) / 4;
  // adjust for increment
  target--;
  // set pc counter to target address
  pc = target;
  return assem;
}

char* bne(void* instruction, int counter) {
  cat_1* instr = (cat_1*) instruction;
  char* assem  = cat1String("bne", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // check if rs1 != rs2, if fails, exit function
  if (registers[instr->rs1] == registers[instr->rs2]) { return assem; }
  // get the current address
  int cur    = (counter * 4) + offset;
  // left shift immediate
  int shift  = instr->imm1 * 2;
  // create target address
  int target = cur + shift;
  // convert target address to index
  target     = (target - offset) / 4;
  // adjust for increment
  target--;
  // set pc counter to target address
  pc = target;
  return assem;
}

char* blt(void* instruction, int counter) {
  cat_1* instr = (cat_1*) instruction;
  char* assem  = cat1String("blt", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // check if rs1 < rs2, if fails, exit function
  if (registers[instr->rs1] >= registers[instr->rs2]) { return assem; }
  // get the current address
  int cur    = (counter * 4) + offset;
  // left shift immediate
  int shift  = instr->imm1 * 2;
  // create target address
  int target = cur + shift;
  // convert target address to index
  target     = (target - offset) / 4;
  // adjust for increment
  target--;
  // set pc counter to target address
  pc = target;
  return assem;
}

char* sw(void* instruction, int counter) {
  cat_1* instr = (cat_1*) instruction;
  char* assem  = malloc(20 * sizeof(char));
  sprintf(assem, "sw x%d, %d(x%d)", instr->rs1, instr->imm1, instr->rs2);
  if (exec == false) { return assem; }
  // add rs1 with imm for target to place rs2 to memory
  int target = registers[instr->rs2] + instr->imm1;
  // convert target to array index
  target     = (target - offset) / 4;
  // free old data
  free((memory[target]->data));
  // create new data to put in memory
  int* value           = malloc(sizeof(int));
  // set the new value of the integer in memory
  *value               = registers[instr->rs1];
  // store the new value in memory
  memory[target]->data = value;
  return assem;
}

// generates assembly string from cat2 instructions
char* cat2String(char* instr, cat_2* instruction) {
  char* assem = malloc(20 * sizeof(char));
  sprintf(assem, "%s x%d, x%d, x%d", instr, instruction->rd, instruction->rs1, instruction->rs2);
  return assem;
}

// Category 2, R-Type instructions
char* add(void* instruction, int counter) {
  cat_2* instr = (cat_2*) instruction;
  char* assem  = cat2String("add", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // rs1 + rs2 = rd
  registers[instr->rd] = registers[instr->rs1] + registers[instr->rs2];
  return assem;
}

char* sub(void* instruction, int counter) {
  cat_2* instr = (cat_2*) instruction;
  char* assem  = cat2String("sub", instr);
  // skip execution if not toggled
  if (exec == false) { return assem; }
  // rs1 - rs2 = rd
  registers[instr->rd] = registers[instr->rs1] - registers[instr->rs2];
  return assem;
}

char* and (void* instruction, int counter) {
  cat_2* instr = (cat_2*) instruction;
  char* assem  = cat2String("and", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // rd = rs1 & rs2
  registers[instr->rd] = registers[instr->rs1] & registers[instr->rs2];
  return assem;
}

char* or (void* instruction, int counter) {
  cat_2* instr = (cat_2*) instruction;
  char* assem  = cat2String("or", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // rd = rs1 | rs2
  registers[instr->rd] = registers[instr->rs1] | registers[instr->rs2];
  return assem;
}

// generates assembly string from cat1 instructions
char* cat3String(char* instr, cat_3* instruction) {
  char* assem = malloc(20 * sizeof(char));
  sprintf(assem, "%s x%d, x%d, #%d", instr, instruction->rd, instruction->rs1, instruction->imm1);
  return assem;
}

// Category 3, I-Type instructions
char* addi(void* instruction, int counter) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = cat3String("addi", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // rd = rs1 + #
  registers[instr->rd] = registers[instr->rs1] + instr->imm1;
  return assem;
}

char* andi(void* instruction, int counter) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = cat3String("andi", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // rd = rs1 & #
  registers[instr->rd] = registers[instr->rs1] & instr->imm1;
  return assem;
}

char* ori(void* instruction, int counter) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = cat3String("ori", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // rd = rs1 | #
  registers[instr->rd] = registers[instr->rs1] | instr->imm1;
  return assem;
}

char* sll(void* instruction, int counter) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = cat3String("sll", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // rd = rs1 << #
  registers[instr->rd] = registers[instr->rs1] << instr->imm1;
  return assem;
}

char* sra(void* instruction, int counter) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = cat3String("sra", instr);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // arithmetic shift preserves sign, most systems impl this as arithmetic shift
  registers[instr->rd] = registers[instr->rs1] >> instr->imm1;
  return assem;
}

char* lw(void* instruction, int counter) {
  cat_3* instr = (cat_3*) instruction;
  char* assem  = malloc(20 * sizeof(char));
  sprintf(assem, "lw x%d, %d(x%d)", instr->rd, instr->imm1, instr->rs1);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // calculate the address
  int address          = registers[instr->rs1] + instr->imm1;
  // convert the address into an index in memory
  address              = (address - offset) / 4;
  //  put the address from memory into the target register
  registers[instr->rd] = *((int*) memory[address]->data);
  return assem;
}

// Category 4, U-Type instructions
char* jal(void* instruction, int counter) {
  cat_4* instr = (cat_4*) instruction;
  char* assem  = malloc(20 * sizeof(char));
  sprintf(assem, "jal x%d, #%d", instr->rd, instr->imm1);
  //  skip execution if not toggled
  if (exec == false) { return assem; }
  // --- store next address ---
  // get the current address
  int cur              = (counter * 4) + offset;
  // store address of next instruction in rd
  registers[instr->rd] = cur + 4;
  // --- jump to next address ---
  // shift the immediate
  int shift            = instr->imm1 * 2;
  // get the jump address
  int jump             = shift + cur;
  // convert the jump address to a memory index
  jump                 = (jump - offset) / 4;
  // adjust for increment
  jump--;
  // set the pc to the jump address
  pc = jump;
  return assem;
}

// will set global program flag to true
// Prevents parsing further code if ran
char* br(void* instruction, int counter) {
  ENDFLAG = true;
  return "break";
} // Can't use "break" as it's a C keyword

// Opcode mapping table
func_type c1[4]       = {beq, bne, blt, sw};
func_type c2[4]       = {add, sub, and, or };
func_type c3[6]       = {addi, andi, ori, sll, sra, lw};
func_type c4[2]       = {jal, br};
func_type* opcodes[4] = {c4, c2, c3, c1};

// parses the file for all the instructions and gens disassembly
void parseFile(FILE* fp) {

  // Init the file reader
  char line[35]; // 32 bit word + \n\r + \0
  memset(line, '\0', 35 * sizeof(char));
  char op[6];
  memset(op, '\0', 6 * sizeof(char));

  // flag for the end of a program
  bool endFlag = false;

  // init the queue
  STAILQ_INIT(&memqueue);

  // Itterate over every line in the file
  while (fgets(line, 35, fp) != NULL) {
    // increase the program size
    programSize++;
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
      // convert binary string to decimal signed integer
      *num       = (int) strtol(line, NULL, 2);
      // store pointer to integer in data field of queue item
      item->data = num;

      // put the int on the memory queue
      STAILQ_INSERT_TAIL(&memqueue, item, next);
      continue;
    }

    // Get the category code
    char catStr[3] = {line[30], line[31], '\0'};
    // convert binary string category to short integer
    short category = (short) strtol(catStr, NULL, 2);

    // --- Process opcodes ---
    strncpy(op, &line[25], 5);              // Get the opcode
    op[5]      = '\0';                      // Terminate Manually
    int opcode = (int) strtol(op, NULL, 2); // convert to decimal

    // 11111 is break, start reading integer values
    if (opcode == 31) {
      opcode  = 1;
      endFlag = true;
    }

    // add category and opcode to queue item
    item->category = category;
    item->opcode   = opcode;
    item->counter  = 0;     // init counter
    item->moved    = false; // init move flag

    // get arguments based on category
    char* im;
    int imm = 0;
    // used to sign extend binary string before conversion
    char signExtend[33];

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
      im = malloc(21 * sizeof(char));
      strncpy(im, line, 20);
      im[20] = '\0';

      // sign extend the imm string
      memset(signExtend, '\0', 33 * sizeof(char));
      if (im[0] == '1') strcat(signExtend, "111111111111");
      strcat(signExtend, im);

      // convert imm to decimal
      imm = (int) strtol(signExtend, NULL, 2);

      // create instruction and insert into item
      cat_4* instruction4 = malloc(sizeof(cat_4));
      instruction4->imm1  = imm;
      instruction4->rd    = rd;

      // add instruction ptr to queue item
      item->data = instruction4;

      // get the assembly output of the command and store for pipelining cycle output
      item->assem = opcodes[item->category][item->opcode](item->data, item->counter);

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

      // get the assembly output of the command and store for pipelining cycle output
      item->assem = opcodes[item->category][item->opcode](item->data, item->counter);

      // place item on queue
      STAILQ_INSERT_TAIL(&memqueue, item, next);
      break;
    // cat 3
    case 2:
      // extract imm from text
      im = malloc(13 * sizeof(char));
      strncpy(im, line, 12);
      im[12] = '\0';

      // sign extend the imm  if needed string
      memset(signExtend, '\0', 33 * sizeof(char));
      if (im[0] == '1') strcat(signExtend, "11111111111111111111");
      strcat(signExtend, im);

      // convert to decimal
      imm = (int) strtol(signExtend, NULL, 2);

      // create instruction and insert into item
      cat_3* instruction3 = malloc(sizeof(cat_3));
      instruction3->imm1  = imm;
      instruction3->rs1   = rs1;
      instruction3->rd    = rd;

      // add instruction ptr to queue item
      item->data = instruction3;

      // get the assembly output of the command and store for pipelining cycle output
      item->assem = opcodes[item->category][item->opcode](item->data, item->counter);

      // place item on queue
      STAILQ_INSERT_TAIL(&memqueue, item, next);
      break;
    // cat 1
    case 3:
      // extract imm1 and imm2 from text
      im = malloc(8 * sizeof(char));
      memset(im, '\0', 8 * sizeof(char));
      strncpy(im, line, 7);
      im[7] = '\0';

      // create string for concating two parts of integer
      char concat[13];
      memset(concat, '\0', 13 * sizeof(char));
      strcat(concat, im);

      // rd location has the rest of the integer
      strcat(concat, reg);

      // sign extend if needed
      memset(signExtend, '\0', 33 * sizeof(char));
      if (im[0] == '1') strcat(signExtend, "11111111111111111111");
      strcat(signExtend, concat);

      imm = (int) strtol(signExtend, NULL, 2);

      // create instruction and insert into item
      cat_1* instruction1 = malloc(sizeof(cat_1));
      instruction1->imm1  = imm;
      instruction1->rs1   = rs1;
      instruction1->rs2   = rs2;

      // add instruction ptr to queue item
      item->data = instruction1;

      // get the assembly output of the command and store for pipelining cycle output
      item->assem = opcodes[item->category][item->opcode](item->data, item->counter);

      // place item on queue
      STAILQ_INSERT_TAIL(&memqueue, item, next);
      break;
    }
  }
  // reset the End Flag for later runs
  ENDFLAG = false;
} // END parseFile()

// parse over the entire array
char* loadQueueToMemory() {
  // dynamically create memory array
  memory = malloc(programSize * sizeof(entry*));
  memset(memory, 0, programSize * sizeof(entry*));

  // itteration item
  entry* item;
  dCounter     = 0;
  // char * # lines * # chars per line
  char* output = malloc(programSize * 57 * sizeof(char));
  STAILQ_FOREACH(item, &memqueue, next) {
    // set pointer to queue item in memory
    memory[pc]   = item;
    // convert the index to an address
    int location = (pc * 4) + offset;
    char line[57];
    // start the array of data entries
    if (ENDFLAG == true) {
      // for initial case
      if (data == NULL) {
        // initalize data array
        data = malloc((programSize - pc) * sizeof(entry*));
        memset(data, 0, (programSize - pc) * sizeof(entry*));
        // store the address of the first instatance of data for printing
        dataStart = location;
      }
      // set copy the pointer from memory
      data[dCounter] = memory[pc];
      // add the dissasembly of data
      sprintf(line, "%s\t%d\t%d\n", item->line, location, *((int*) item->data));
      strcat(output, line);
      // increase the counter for data and program counter
      dCounter++;
      pc++;
      continue;
    }
    // call operation to get the instruction at address in memory
    char* assem = opcodes[item->category][item->opcode](item->data, item->counter);
    // add the disasembly to the output
    sprintf(line, "%s\t%d\t%s\n", item->line, location, assem);
    strcat(output, line);
    // increment the program counter
    pc++;
  }
  // reset for next run
  ENDFLAG = false;
  pc      = 0;
  // return the disassembly
  return output;
}

// take cycle information and format a string output
char* printCycle() {
// I will commit heresey >:D
#define hySize         22
#define ifUnitCharSize 11
#define unitSize       55
#define registerSize   438
#define dataWordSize   113
  char hypens[hySize] = "--------------------\n";
  char header[unitSize]; // cycle header
  sprintf(header, "Cycle %d:\n", cycle);

  // --- Print Pipeline ---

  // --- IF Unit ---
  char ifUnit[ifUnitCharSize] = "\nIF Unit:\n";
  char ifWait[unitSize];
  char ifExec[unitSize];

  // write IF Unit WAITING state
  if (IFUnitWait != NULL) sprintf(ifWait, "\tWaiting: [%s]\n", IFUnitWait->assem);
  else sprintf(ifWait, "\tWaiting:\n");

  // write IF Unit EXEC state
  if (IFUnitExecuted != NULL) sprintf(ifExec, "\tExecuted:[%s]\n", IFUnitExecuted->assem);
  else sprintf(ifExec, "\tExecuted:\n");

  // --- Pre-Issue Queue ---
  char preIssQ[hySize] = "Pre-Issue Queue:\n";
  char issues[PRE_ISSUE_QUEUE_LIMIT][unitSize];
  memset(issues, PRE_ISSUE_QUEUE_LIMIT * unitSize, sizeof(char));

  // get the first item
  entry* item = STAILQ_FIRST(&preIssueQueue);

  // itterate over the queue
  for (int i = 0; i < PRE_ISSUE_QUEUE_LIMIT; i++) {

    // if the spot in the queue is filled...
    if (item != NULL) {

      // Write the line
      sprintf(issues[i], "\tEntry %d: [%s]\n", i, item->assem);

      // goto next item
      item = STAILQ_NEXT(item, next);

      // next itteration
      continue;
    }
    // case when item is NULL
    sprintf(issues[i], "\tEntry %d:\n", i);

  } // END FOR-LOOP

  // --- Pre-ALU1 Queue ---
  // ALU title
  char preAlu1[hySize] = "Pre-ALU1 Queue:\n";
  char alu1[PRE_ALU1_QUEUE_LIMIT][unitSize];

  // get first item of ALU1 Queue
  item = STAILQ_FIRST(&preALU1Queue);

  // Itterate over ALU1 Queue
  for (int i = 0; i < PRE_ALU1_QUEUE_LIMIT; i++) {

    // check if queue item exists
    if (item != NULL) {

      // Write line
      sprintf(alu1[i], "\tEntry %d: [%s]\n", i, item->assem);

      // goto next item
      item = STAILQ_NEXT(item, next);

      // next itteration
      continue;
    }
    // case when item is NULL
    sprintf(alu1[i], "\tEntry %d:\n", i);

  } // END FOR-LOOP

  // --- Pre-Mem Queue ---
  char preMemTitle[hySize] = "Pre-MEM Queue: ";
  char preMemStr[unitSize];

  // write pre-mem queue
  if (preMEMQueue != NULL) sprintf(preMemStr, "[%s]\n", preMEMQueue->assem);
  else sprintf(preMemStr, "\n");

  // --- Post-Mem Queue ---
  char postMemTitle[hySize] = "Post-MEM Queue: ";
  char postMemStr[unitSize];

  // write pre-mem queue
  if (postMEMQueue != NULL) sprintf(postMemStr, "[%s]\n", postMEMQueue->assem);
  else sprintf(postMemStr, "\n");

  // --- Pre-ALU2 Queue ---
  // ALU2 title
  char preAlu2[hySize] = "Pre-ALU2 Queue:\n";
  // get first item of ALU2 Queue
  char alu2[PRE_ALU2_QUEUE_LIMIT][unitSize];
  item = STAILQ_FIRST(&preALU2Queue);

  // Itterate over ALU2 Queue
  for (int i = 0; i < PRE_ALU2_QUEUE_LIMIT; i++) {

    // check if queue item exists
    if (item != NULL) {

      // Write line
      sprintf(alu2[i], "\tEntry %d: [%s]\n", i, item->assem);

      // goto next item
      item = STAILQ_NEXT(item, next);

      // next itteration
      continue;
    }
    // case when item is NULL
    sprintf(alu2[i], "\tEntry %d:\n", i);

  } // END FOR-LOOP

  // --- Post-ALU2 Queue ---

  // --- Print Registers and Data ---
  char regs[registerSize] = "Registers\n"; // contains all registers, 107*4 + 10=428
  char r[107];                             // temporary var for each line of registers
  // 11 max characters * 8 per line, + 8 tabs + 5 for start of line + 1 terminating + 5 for good luck = 107
  memset(r, '\0', 107 * sizeof(char));
  // itterate over all registers
  for (int i = 0; i < 4; i++) {
    int start = i * 8;
    sprintf(r, "x%02d:\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", start, registers[start], registers[start + 1],
            registers[start + 2], registers[start + 3], registers[start + 4], registers[start + 5], registers[start + 6],
            registers[start + 7]);
    strcat(regs, r);
  }
  // print out data line
  // (max range is 11 characters per int * 8 per line + 12 max possible address string + 8 tabs + 5 gl) * (number of data
  // addresses / 8) + 10 for good luck
  int numLines   = (int) (dCounter / 8) + 1;            // number of lines needed
  int addr       = dataStart;                           // start address of data words
  int totalChars = (dataWordSize * numLines) + 10;      // total number of chars need for data
  char dataWords[totalChars];                           // store the data ints
  memset(dataWords, '\0', (totalChars * sizeof(char))); // clear data
  strcat(dataWords, "Data");                            // add data header
  char d[dataWordSize];                                 // used for sprintf
  memset(d, '\0', dataWordSize * sizeof(char));

  // itterate over all data words
  for (int j = 0; j < dCounter; j++) {
    // add a newline with address every 8 words
    if (j % 8 == 0) {
      sprintf(d, "\n%d:", addr);
      strcat(dataWords, d);
      addr += 32;
    }
    // add new value
    sprintf(d, "\t%d", *((int*) data[j]->data));
    strcat(dataWords, d);
  }
  strcat(dataWords, "\n");

  // --- Combine All Strings ---
  int total = hySize + ifUnitCharSize + unitSize + unitSize + hySize + (PRE_ISSUE_QUEUE_LIMIT * unitSize) + hySize
              + (PRE_ALU1_QUEUE_LIMIT * unitSize) + hySize + unitSize + hySize + unitSize + hySize
              + (PRE_ALU2_QUEUE_LIMIT * unitSize) + unitSize + registerSize + dataWordSize + totalChars;
  char* output = malloc(total * sizeof(char));    // create output variable
  memset(output, '\0', total * sizeof(char));     // clear memory
  strcat(output, hypens);                         // Hypens
  strcat(output, header);                         // cycle header
  strcat(output, ifUnit);                         // IF Unit title
  strcat(output, ifWait);                         // IF Unit wait state
  strcat(output, ifExec);                         // IF Unit Exec state
  strcat(output, preIssQ);                        // Pre-Issue Queue Title
  for (int i = 0; i < PRE_ISSUE_QUEUE_LIMIT; i++) //
    strcat(output, issues[i]);                    // Pre-Issue Queue Item i
  strcat(output, preAlu1);                        // ALU1 title
  for (int i = 0; i < PRE_ALU1_QUEUE_LIMIT; i++)  //
    strcat(output, alu1[i]);                      // Pre-ALU1 Queue Item i
  strcat(output, preMemTitle);                    // Pre-mem title
  strcat(output, preMemStr);                      // Pre-mem content
  strcat(output, postMemTitle);                   // Post-mem title
  strcat(output, postMemStr);                     // Post-mem content
  strcat(output, preAlu2);                        // ALU2 title
  for (int i = 0; i < PRE_ALU2_QUEUE_LIMIT; i++)  //
    strcat(output, alu2[i]);                      // Pre-ALU2 Queue Item i
  strcat(output, regs);                           // Registers
  strcat(output, dataWords);                      // int data
  return output;
}

// copy target information without pointing to next entry
entry* copyEntry(entry* target) {
  entry* output    = malloc(sizeof(entry));
  output->data     = target->data;
  output->category = target->category;
  output->opcode   = target->opcode;
  output->line     = target->line;
  output->assem    = target->assem;
  output->counter  = target->counter;
  output->moved    = target->moved;

  return output;
}

// checks target entry for NULL before resetting move flag
void safeMoveReset(entry* target) {
  if (target != NULL) target->moved = false;
}

// reset moved flag in all instructions in all queues
void resetMoveFlags() {
  // itteration object
  entry* item;
  // itterate over all queue objects and set moved to false
  STAILQ_FOREACH(item, &memqueue, next) item->moved      = false;
  STAILQ_FOREACH(item, &preIssueQueue, next) item->moved = false;
  STAILQ_FOREACH(item, &preALU1Queue, next) item->moved  = false;
  STAILQ_FOREACH(item, &preALU2Queue, next) item->moved  = false;

  // manually set the move flag to false for each single entry
  safeMoveReset(IFUnitWait);
  safeMoveReset(IFUnitExecuted);
  safeMoveReset(preMEMQueue);
  safeMoveReset(postMEMQueue);
  safeMoveReset(postALU2Queue);
}

// stalled at end of last cycle
void instructionFetchUnit() {

  // exit Unit if halted
  if (IF_HALT == true) { return; }

  // --- IF STATE CHECKS ---
  // check if the Fetch Unit is waiting
  bool IS_WAITING = (IFUnitWait != NULL);

  // check if the Fetch Unit needs to EXEC something
  bool IS_EXEC = (IFUnitExecuted != NULL);

  // check for ALU2 instr in pipeline
  bool ACTIVE_ALU2 = (numALU2instr > 0);

  // check if the pre-issue queue is full
  bool ISSUE_QUEUE_FULL = (preIssueQueueSize >= PRE_ISSUE_QUEUE_LIMIT);

  // --- GET NEXT INSTRUCTION ---
  // init an empty instruction to fetch
  entry* instruction;

  // fetch 2 instructions, if possible
  for (int i = 0; i < 2; i++) {

    // normal fetching routine
    if (!IS_WAITING && !IS_EXEC) {
      instruction          = copyEntry(memory[pc]);
      // store the current state of the PC
      instruction->counter = pc;
    }
    // execution if any
    else if (IS_EXEC)
      instruction = IFUnitExecuted;
    // unit is waiting on an instruction
    else instruction = IFUnitWait;

    // instruction has been loaded, set moved to true
    instruction->moved = true;

    // --- WORKING INSTRUCTION STATE CHECKS ---
    // check if the fetched instruction is a branch instruction
    bool IS_BRANCH_INSTR = (instruction->category == 3 && instruction->opcode < 3)
                           || (instruction->category == 0 && instruction->opcode == 0);

    // check if the instruction is a break instruction
    bool IS_BREAK = (instruction->category == 0 && instruction->opcode == 1);

    // check if the instruction is an ALU2 instruction
    bool IS_ALU2_INSTR = (instruction->category == 1) || (instruction->category == 2 && instruction->category < 5);

    // --- MOVE TO WAIT CONDITIONS ---
    if ((IS_BRANCH_INSTR && ACTIVE_ALU2) || ISSUE_QUEUE_FULL || ACTIVE_ALU2) {
      IFUnitWait = instruction;
      return;
    }

    // --- MOVE TO EXEC CONDITIONS ---
    if ((!ACTIVE_ALU2 && (IS_BRANCH_INSTR)) || IS_BREAK) {
      // send instruction to EXEC from waiting state
      if (IS_WAITING) {
        IFUnitExecuted = IFUnitWait;
        IFUnitWait     = NULL; // clear waiting state
        return;
      }
      // send instruction directly to EXEC
      IFUnitExecuted = instruction;
      return;
    }

    // --- EXEC CONDITIONS ---
    if (IS_EXEC) {
      int cat     = instruction->category;
      int op      = instruction->opcode;
      char* instr = opcodes[cat][op](instruction->data, instruction->counter);

      // stop IF Unit if Break is read
      if (IS_BREAK) { IF_HALT = true; }

      // clear execution state
      IFUnitExecuted = NULL;
    }

    // --- FETCH INSTRUCTION ---

    // Increment number of ALU2 instructions in pipeline
    if (IS_ALU2_INSTR) { numALU2instr++; }

    // push the instruction onto the pre-issue
    STAILQ_INSERT_TAIL(&preIssueQueue, instruction, next);

    // increment the size of the queue
    preIssueQueueSize++;

    // increment PC
    pc++;
  } // END FOR LOOP

  // IF Unit finished, exit
  return;
} // END instructionFetchUnit()

// execute the program a cycle at a time
void executeProgram() {
  // make sure all settings are zero/initalized and cleared
  pc      = 0;                            // reset program counter to 0
  cycle   = 1;                            // tracks current cycle
  ENDFLAG = false;                        // marks end of program
  exec    = true;                         // set to true to set functions in execute mode
  memset(registers, 0, 32 * sizeof(int)); // set all registers to 0
  STAILQ_INIT(&cycleQueue);               // Init the cycle queue
  STAILQ_INIT(&preIssueQueue);            // Init the cycle queue
  STAILQ_INIT(&preALU1Queue);             // Init the cycle queue
  STAILQ_INIT(&preALU2Queue);             // Init the cycle queue

  int cat = 0;
  int op  = 0;
  // --- begin program execution ---
  while (ENDFLAG == false) {
    // create new entry to hold cycle information
    entry* item = malloc(sizeof(entry));
    // --- FETCH UNIT ---
    instructionFetchUnit();
    // get the instruction from the current place in memory
    entry* instruction = memory[pc];
    // calculate the address
    int address        = (pc * 4) + offset;
    // run the instruction
    cat                = instruction->category;
    op                 = instruction->opcode;
    // char* instr        = opcodes[cat][op](instruction->data, instruction->counter);
    //  print the cycle to line structure
    item->line         = printCycle();
    //  add cycle entry to queue
    STAILQ_INSERT_TAIL(&cycleQueue, item, next);
    // go to next instruction and increase the cycle
    // pc++;
    cycle++;
    // reset move flags at end of cycle
    resetMoveFlags();
  }
}

void printToFile() {
  // create simulation file
  FILE* simOut = fopen("simulation.txt", "w");
  entry* item;
  // itterate over every cycle queue item
  STAILQ_FOREACH(item, &cycleQueue, next) {
    // print the item to the file
    fprintf(simOut, "%s", item->line);
  }
  // close the file when done writing
  fclose(simOut);
}