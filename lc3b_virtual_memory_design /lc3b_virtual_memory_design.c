/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {
    IRD,
    COND3,COND2,COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    LD_EXCT,
    LD_PSR15,
    LD_Vector,
    LD_SSP,
    LD_USP,
    LD_VA,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    GATE_PC_2,
    GATE_PSR,
    GATE_Vector,
    GATE_SP,
    GATE_VA,
    PCMUX1, PCMUX0,
    DRMUX1, DRMUX0,
    SR1MUX1, SR1MUX0,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    PSR1MUX,
    PSR2MUX,
    EXCTMUX1,EXCTMUX0,
    SPMUX1,SPMUX0,
    VectorMUX2,VectorMUX1,VectorMUX0,
    VAMUX,
    MDRMUX1,MDRMUX0,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    /* MODIFY: you have to add all your new control signals */
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND3] << 3)+(x[COND2] << 2)+(x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
                                      (x[J3] << 3) + (x[J2] << 2) +
                                      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetLD_EXCT(int *x)       { return(x[LD_EXCT]);}
int GetLD_PSR15(int *x)      { return(x[LD_PSR15]);}
int GetLD_Vector(int *x)     { return(x[LD_Vector]);}
int GetLD_SSP(int *x)        { return(x[LD_SSP]);}
int GetLD_USP(int *x)        { return(x[LD_USP]);}
int GetLD_VA(int *x)         { return(x[LD_VA]);}

int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetGATE_PC_2(int *x)     { return(x[GATE_PC_2]); }
int GetGATE_PSR(int *x)      { return(x[GATE_PSR]); }
int GetGATE_Vector(int *x)   { return(x[GATE_Vector]); }
int GetGATE_SP(int *x)       { return(x[GATE_SP]); }
int GetGATE_VA(int *x)       { return(x[GATE_VA]); }

int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return((x[DRMUX1] << 1) + x[DRMUX0]); }
int GetSR1MUX(int *x)        { return((x[SR1MUX1] << 1) + x[SR1MUX0]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetPSR1MUX(int *x)       { return(x[PSR1MUX]); }
int GetPSR2MUX(int *x)       { return(x[PSR2MUX]); }
int GetEXCTMUX(int *x)       { return((x[EXCTMUX1] << 1) + x[EXCTMUX0]);}

int GetSPMUX(int *x)         { return((x[SPMUX1] << 1) + x[SPMUX0]); }
int GetVectorMUX(int *x)     { return((x[VectorMUX2] << 2)+(x[VectorMUX1] << 1) + x[VectorMUX0]); }
int GetVAMUX(int *x)         { return(x[VAMUX]); }
int GetMDRMUX(int *x)        { return((x[MDRMUX1] << 1) + x[MDRMUX0]);}

int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); }
int GetLSHF1(int *x)         { return(x[LSHF1]); }

/* MODIFY: you can add more Get functions for your new control signals */

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
 MEMORY[A][1] stores the most significant byte of word at word address A
 There are two write enable signals, one for each byte. WE0 is used for
 the least significant byte of a word. WE1 is used for the most significant
 byte of a word. */

#define WORDS_IN_MEM    0x2000 /* 32 frames */
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{
    
    int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */
    
    int READY;	/* ready bit */
    /* The ready bit is also latched as you dont want the memory system to assert it
     at a bad point in the cycle*/
    
    int REGS[LC_3b_REGS]; /* register file. */
    
    int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */
    
    int STATE_NUMBER; /* Current State Number - Provided for debugging */
    
    /* For lab 4 */
    int INTV; /* Interrupt vector register */
    int EXCV; /* Exception vector register */
    int SSP; /* Initial value of system stack pointer */
    /* MODIFY: You may add system latches that are required by your implementation */
    int PSR;
    int EXCT;
    int Vector;
    int USP;
    
    /* For lab 5 */
    int PTBR; /* This is initialized when we load the page table */
    int VA;   /* Temporary VA register */
    /* MODIFY: you should add here any other registers you need to implement virtual memory */
    
    
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/* For lab 5 */
#define PAGE_NUM_BITS 9
#define PTE_PFN_MASK 0x3E00
#define PTE_VALID_MASK 0x0004
#define PAGE_OFFSET_MASK 0x1FF

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;
int INT=0;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {
    
    eval_micro_sequencer();
    cycle_memory();
    eval_bus_drivers();
    drive_bus();
    latch_datapath_values();
    
    CURRENT_LATCHES = NEXT_LATCHES;
    
    CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {
    int i;
    
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }
    
    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
        if (CURRENT_LATCHES.PC == 0x0000) {
            RUN_BIT = FALSE;
            printf("Simulator halted\n\n");
            break;
        }
        cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {
    if (RUN_BIT == FALSE) {
        printf("Can't simulate, Simulator is halted\n\n");
        return;
    }
    
    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
        cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {
    int address; /* this is a byte address */
    
    printf("\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        printf("  0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");
    
    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%0.4x..0x%0.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
        fprintf(dumpsim_file, " 0x%0.4x (%d) : 0x%0.2x%0.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {
    int k;
    
    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%0.4x\n", BUS);
    printf("MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    /*printf("CURRENT PSR: 0x%0.4x\n",CURRENT_LATCHES.PSR);
     printf("CURRENT VECTOR: 0x%0.4x\n",CURRENT_LATCHES.Vector);*/
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        printf("%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");
    
    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%0.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%0.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%0.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%0.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%0.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%0.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
        fprintf(dumpsim_file, "%d: 0x%0.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    /*fprintf(dumpsim_file, "x4000: 0x%0.4x\n",((MEMORY[0x2000][1]<<8)+MEMORY[0X2000][0]));*/
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {
    char buffer[20];
    int start, stop, cycles;
    
    printf("LC-3b-SIM> ");
    
    scanf("%s", buffer);
    printf("\n");
    
    switch(buffer[0]) {
        case 'G':
        case 'g':
            go();
            break;
            
        case 'M':
        case 'm':
            scanf("%i %i", &start, &stop);
            mdump(dumpsim_file, start, stop);
            break;
            
        case '?':
            help();
            break;
        case 'Q':
        case 'q':
            printf("Bye.\n");
            exit(0);
            
        case 'R':
        case 'r':
            if (buffer[1] == 'd' || buffer[1] == 'D')
                rdump(dumpsim_file);
            else {
                scanf("%d", &cycles);
                run(cycles);
            }
            break;
            
        default:
            printf("Invalid Command\n");
            break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {
    FILE *ucode;
    int i, j, index;
    char line[200];
    
    printf("Loading Control Store from file: %s\n", ucode_filename);
    
    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
        printf("Error: Can't open micro-code file %s\n", ucode_filename);
        exit(-1);
    }
    
    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
        if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
            printf("Error: Too few lines (%d) in micro-code file: %s\n",
                   i, ucode_filename);
            exit(-1);
        }
        
        /* Put in bits one at a time. */
        index = 0;
        
        for (j = 0; j < CONTROL_STORE_BITS; j++) {
            /* Needs to find enough bits in line. */
            if (line[index] == '\0') {
                printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
                       ucode_filename, i);
                exit(-1);
            }
            if (line[index] != '0' && line[index] != '1') {
                printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
                       ucode_filename, i, j);
                exit(-1);
            }
            
            /* Set the bit in the Control Store. */
            CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
            index++;
        }
        
        /* Warn about extra bits in line. */
        if (line[index] != '\0')
            printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
                   ucode_filename, i);
    }
    printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory() {
    int i;
    
    for (i=0; i < WORDS_IN_MEM; i++) {
        MEMORY[i][0] = 0;
        MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename, int is_virtual_base) {
    FILE * prog;
    int ii, word, program_base, pte, virtual_pc;
    
    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
        printf("Error: Can't open program file %s\n", program_filename);
        exit(-1);
    }
    
    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
        program_base = word >> 1;
    else {
        printf("Error: Program file is empty\n");
        exit(-1);
    }
    
    if (is_virtual_base) {
        if (CURRENT_LATCHES.PTBR == 0) {
            printf("Error: Page table base not loaded %s\n", program_filename);
            exit(-1);
        }
        
        /* convert virtual_base to physical_base */
        virtual_pc = program_base << 1;
        pte = (MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][1] << 8) |
        MEMORY[(CURRENT_LATCHES.PTBR + (((program_base << 1) >> PAGE_NUM_BITS) << 1)) >> 1][0];
        
        printf("virtual base of program: %04x\npte: %04x\n", program_base << 1, pte);
        if ((pte & PTE_VALID_MASK) == PTE_VALID_MASK) {
            program_base = (pte & PTE_PFN_MASK) | ((program_base << 1) & PAGE_OFFSET_MASK);
            printf("physical base of program: %x\n\n", program_base);
            program_base = program_base >> 1;
        } else {
            printf("attempting to load a program into an invalid (non-resident) page\n\n");
            exit(-1);
        }
    }
    else {
        /* is page table */
        CURRENT_LATCHES.PTBR = program_base << 1;
    }
    
    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
        /* Make sure it fits. */
        if (program_base + ii >= WORDS_IN_MEM) {
            printf("Error: Program file %s is too long to fit in memory. %x\n",
                   program_filename, ii);
            exit(-1);
        }
        
        /* Write the word to memory array. */
        MEMORY[program_base + ii][0] = word & 0x00FF;
        MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;;
        ii++;
    }
    
    if (CURRENT_LATCHES.PC == 0 && is_virtual_base)
        CURRENT_LATCHES.PC = virtual_pc;
    
    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */
/*             and set up initial state of the machine         */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *pagetable_filename, char *program_filename, int num_prog_files) {
    int i;
    init_control_store(ucode_filename);
    
    init_memory();
    load_program(pagetable_filename,0);
    for ( i = 0; i < num_prog_files; i++ ) {
        load_program(program_filename,1);
        while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);
    CURRENT_LATCHES.SSP = 0x3000; /* Initial value of system stack pointer */
    
    /* MODIFY: you can add more initialization code HERE */
    
    NEXT_LATCHES = CURRENT_LATCHES;
    
    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {
    FILE * dumpsim_file;
    
    /* Error Checking */
    if (argc < 4) {
        printf("Error: usage: %s <micro_code_file> <page table file> <program_file_1> <program_file_2> ...\n",
               argv[0]);
        exit(1);
    }
    
    printf("LC-3b Simulator\n\n");
    
    initialize(argv[1], argv[2], argv[3], argc - 3);
    
    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
        printf("Error: Can't open dumpsim file\n");
        exit(-1);
    }
    
    while (1)
        get_command(dumpsim_file);
    
}

/***************************************************************/
/* Do not modify the above code, except for the places indicated
 with a "MODIFY:" comment.
 You are allowed to use the following global variables in your
 code. These are defined above.
 
 CONTROL_STORE
 MEMORY
 BUS
 
 CURRENT_LATCHES
 NEXT_LATCHES
 
 You may define your own local/global variables and functions.
 You may use the functions to get at the control bits defined
 above.
 
 Begin your code here 	  			       */
/***************************************************************/
int mem_cycle=0;

int IR_signal=0;

int GetIR15_12(int x){
    return ((x&0xF000)>>12);
}

int GetDR(int x){
    if (GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION)==0)
        return ((x&0x0E00)>>9);
    if (GetDRMUX(CURRENT_LATCHES.MICROINSTRUCTION)==1)
        return 7;
    else return 6;
}

int GetSR1(int x){
    if(GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)==0)
        return ((x&0x0E00)>>9);
    if(GetSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)==1)
        return ((x&0x01C0)>>6);
    else return 6;
    
}

int GetSR2(int x){
    return(x&0x0007);
}

int GetCOND0(int *x){
    
    return x[COND0];
    
}

int GetCOND1(int *x){
    return x[COND1];
    
}
int OperateJ(int x){
    /*
     int a =GetCOND1(CURRENT_LATCHES.MICROINSTRUCTION);
     int b =GetCOND0(CURRENT_LATCHES.MICROINSTRUCTION);
     int operatenum=((a&(~b)&CURRENT_LATCHES.BEN)<<2) +(((~a)&b&CURRENT_LATCHES.READY)<<1)
     +(a&b&((CURRENT_LATCHES.IR&0x0800)>>11));
     return (x|operatenum);
     */
    if (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION)==0) {
        return x;
    }
    if (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION)==1) {
        if (CURRENT_LATCHES.READY==1)
            return (x+2);
        else return x;
    }
    if (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION)==2) {
        if (CURRENT_LATCHES.BEN==1)
            return (x+4);
        else return x;
    }
    if (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION)==3) {
        if (((CURRENT_LATCHES.IR&0x0800)>>11)==1)
            return (x+1);
        else return x;
    }
    if (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION)==4) {
        if (INT==1)
        {
            INT=0;
            return (x+8);
        }
        else return x;
    }
    else return x;
    
}

int sext(int a , int b){
    int tem = 1<<b;
    int c=0;
    int d=1;
    int i = 0;
    for (i=b;i<16;i++)
        c=c+(1<<i);
    for (i=1;i<b;i++)
        d=d+(1<<i);
    
    if (a&tem) {
        return (a|c);
    } else {
        return (a&d);
    }
}

int GetADDR1value(){
    if (GetADDR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
        return (CURRENT_LATCHES.REGS[GetSR1(CURRENT_LATCHES.IR)]);
    }
    else return (CURRENT_LATCHES.PC);
}

int GetADDR2value(){
    
    int ADDR2MUXnum=0;
    if (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION)==0) {
        ADDR2MUXnum=0;
    }
    if (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION)==1) {
        ADDR2MUXnum=sext(CURRENT_LATCHES.IR,5);    }
    if (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION)==2) {
        ADDR2MUXnum=sext(CURRENT_LATCHES.IR,8);
    }
    if (GetADDR2MUX(CURRENT_LATCHES.MICROINSTRUCTION)==3) {
        ADDR2MUXnum=sext(CURRENT_LATCHES.IR,10);
    }
    if (GetLSHF1(CURRENT_LATCHES.MICROINSTRUCTION)) {
        ADDR2MUXnum=ADDR2MUXnum<<1;
    }
    return Low16bits(ADDR2MUXnum);
}


int GetMARMUXvalue(){
    
    int MARMUXnum;
    
    
    if (GetMARMUX( CURRENT_LATCHES.MICROINSTRUCTION)) {
        MARMUXnum=Low16bits( GetADDR1value()+GetADDR2value());
        return (MARMUXnum);
    }
    else MARMUXnum=((CURRENT_LATCHES.IR&0x00FF)<<1);
    
    return (Low16bits(MARMUXnum));
}

int GetPCvalue(){
    
    return CURRENT_LATCHES.PC;
}

int GetSHFvalue(){
    int a= (CURRENT_LATCHES.IR&0x000F);
    int b = (0xFFFF<<(16-a));
    if ((CURRENT_LATCHES.IR&0x0010)==0) {
        return (Low16bits( CURRENT_LATCHES.REGS[GetSR1(CURRENT_LATCHES.IR)]<<a));
        
    }
    if ((CURRENT_LATCHES.IR&0x0020)==0){
        return (Low16bits(CURRENT_LATCHES.REGS[GetSR1(CURRENT_LATCHES.IR)]>>a));
        
    }
    return (Low16bits((CURRENT_LATCHES.REGS[GetSR1(CURRENT_LATCHES.IR)]>>a)|b));
    
}

int GetMDRvalue(){
    if(GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION))
        return CURRENT_LATCHES.MDR;
    else if (CURRENT_LATCHES.MAR&0x0001) /*odd memory address*/
        return sext(((CURRENT_LATCHES.MDR>>8)&0xFF),7);
    else /*even memory address*/
        return sext((CURRENT_LATCHES.MDR&0xFF),7);
}

int GetALUvalue(){
    int a = CURRENT_LATCHES.REGS[GetSR1(CURRENT_LATCHES.IR)];
    
    int b;
    
    if (CURRENT_LATCHES.IR&0x0020)
        b=sext((CURRENT_LATCHES.IR&0x001F),4);
    else {b = CURRENT_LATCHES.REGS[GetSR2(CURRENT_LATCHES.IR)];}
    if (GetALUK(CURRENT_LATCHES.MICROINSTRUCTION)==0) {
        return Low16bits(a+b);
    }
    if (GetALUK(CURRENT_LATCHES.MICROINSTRUCTION)==1) {
        return Low16bits(a&b);
    }
    if (GetALUK(CURRENT_LATCHES.MICROINSTRUCTION)==2) {
        return Low16bits(a^b);
    }
    else
        return a;
    
    
}

int GetSPvalue(){
    
    if (GetSPMUX(CURRENT_LATCHES.MICROINSTRUCTION)==0) {
        return CURRENT_LATCHES.SSP;
    }
    if (GetSPMUX(CURRENT_LATCHES.MICROINSTRUCTION)==1) {
        return (CURRENT_LATCHES.REGS[GetSR1(CURRENT_LATCHES.IR)]+2);
    }
    if (GetSPMUX(CURRENT_LATCHES.MICROINSTRUCTION)==2) {
        return (CURRENT_LATCHES.REGS[GetSR1(CURRENT_LATCHES.IR)]-2);
    }
    if (GetSPMUX(CURRENT_LATCHES.MICROINSTRUCTION)==3) {
        return CURRENT_LATCHES.USP;
    }
    else return 0;
}
int GetVAvalue(){
    if (GetVAMUX(CURRENT_LATCHES.MICROINSTRUCTION)==0) {
        return ((CURRENT_LATCHES.VA>>9)*2+0x1000);
    }
    else return ((CURRENT_LATCHES.VA&0x01FF)+(CURRENT_LATCHES.MDR&0x3E00));

}

void eval_micro_sequencer() {
    
    /*
     * Evaluate the address of the next state according to the
     * micro sequencer logic. Latch the next microinstruction.
     */
    if (CYCLE_COUNT==300) {
        
        INT=1;
        
    }
    /*
     if (CURRENT_LATCHES.STATE_NUMBER==26) {
     printf("%d\n",CYCLE_COUNT);
     }*/
    
    /*
     if (CYCLE_COUNT==400) {
     for (j=0x100; j<0x104; j++) {
     printf("memory info: 0x%0.4x: 0x%0.4x:\n", j*2,((MEMORY[j][1]<<8)+MEMORY[j][0]));
     }
     for (j=0x600; j<0x60E; j++) {
     printf("memory info: 0x%0.4x: 0x%0.4x:\n", j*2,((MEMORY[j][1]<<8)+MEMORY[j][0]));
     }
     for (j=0x1000; j<0x1001; j++) {
     printf("memory info: 0x%0.4x: 0x%0.4x:\n", j*2,((MEMORY[j][1]<<8)+MEMORY[j][0]));
     }
     for (j=0x6000; j<0x6007; j++) {
     printf("memory info: 0x%0.4x: 0x%0.4x:\n", j*2,((MEMORY[j][1]<<8)+MEMORY[j][0]));
     }
     
     }
     if (CYCLE_COUNT==362) {
     
     printf("memory info: 0x2FFC: 0x%0.4x:\n",((MEMORY[0x17FE][1]<<8)+MEMORY[0x17FE][0]));
     printf("memory info: 0x2FFA: 0x%0.4x:\n",((MEMORY[0x17FD][1]<<8)+MEMORY[0x17FD][0]));
     printf("memory info: 0x2FF8: 0x%0.4x:\n",((MEMORY[0x17FC][1]<<8)+MEMORY[0x17FC][0]));
     
     
     
     }
     if (CYCLE_COUNT==459) {
     
     printf("memory info: 0x2FFC: 0x%0.4x:\n",((MEMORY[0x17FE][1]<<8)+MEMORY[0x17FE][0]));
     printf("memory info: 0x2FFA: 0x%0.4x:\n",((MEMORY[0x17FD][1]<<8)+MEMORY[0x17FD][0]));
     printf("memory info: 0x2FF8: 0x%0.4x:\n",((MEMORY[0x17FC][1]<<8)+MEMORY[0x17FC][0]));
     
     
     
     }
     */
    
    int i = 0;
    int judge=CURRENT_LATCHES.STATE_NUMBER;
    if (judge==2||judge==6||judge==7||judge==3){
        
        IR_signal=1;
        
    }
    if (judge==18||judge==19) {
        
        IR_signal=0;
        
    }
    if (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION)==9) {
        if (IR_signal==0) {
            NEXT_LATCHES.STATE_NUMBER=33;
        }
        else
            NEXT_LATCHES.STATE_NUMBER=GetIR15_12(CURRENT_LATCHES.IR)+34;
    }
    
    else if (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION)==8) {
        if ((CURRENT_LATCHES.IR&0x1000)&IR_signal) {
            
            NEXT_LATCHES.STATE_NUMBER=59;
        }
        else NEXT_LATCHES.STATE_NUMBER=60;
    }
    else if (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION)==7) {
        if (CURRENT_LATCHES.EXCT) {
            
            NEXT_LATCHES.STATE_NUMBER=34;
            
        }
        else NEXT_LATCHES.STATE_NUMBER=OperateJ(GetJ(CURRENT_LATCHES.MICROINSTRUCTION));
    }
    else if (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION)==6) {
        if (CURRENT_LATCHES.EXCT) {
            
            NEXT_LATCHES.STATE_NUMBER=43;
            
        }
        else NEXT_LATCHES.STATE_NUMBER=OperateJ(GetJ(CURRENT_LATCHES.MICROINSTRUCTION));
    }
    else if (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION)==5) {
        if (CURRENT_LATCHES.EXCT) {
            
            NEXT_LATCHES.STATE_NUMBER=46;
            
        }
        else NEXT_LATCHES.STATE_NUMBER=OperateJ(GetJ(CURRENT_LATCHES.MICROINSTRUCTION));
    }
    else if (GetCOND(CURRENT_LATCHES.MICROINSTRUCTION)==0) {
        if (GetIRD(CURRENT_LATCHES.MICROINSTRUCTION)) {
            NEXT_LATCHES.STATE_NUMBER=GetIR15_12(CURRENT_LATCHES.IR);
        }
        else NEXT_LATCHES.STATE_NUMBER=OperateJ(GetJ(CURRENT_LATCHES.MICROINSTRUCTION));
    }
    
    else NEXT_LATCHES.STATE_NUMBER=OperateJ(GetJ(CURRENT_LATCHES.MICROINSTRUCTION));
    
    for (i=0;i<CONTROL_STORE_BITS;i++)
    {
        
        NEXT_LATCHES.MICROINSTRUCTION[i] = CONTROL_STORE[NEXT_LATCHES.STATE_NUMBER][i];
    }
    
    
}


void cycle_memory() {
    
    /*
     * This function emulates memory and the WE logic.
     * Keep track of which cycle of MEMEN we are dealing with.
     * If fourth, we need to latch Ready bit at the end of
     * cycle to prepare microsequencer for the fifth cycle.
     */
    if (CURRENT_LATCHES.READY)
    {
        NEXT_LATCHES.READY=0;
        
    }
    
    if (GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION))
    {
        mem_cycle=mem_cycle+1;
        if (mem_cycle==4)
        {
            NEXT_LATCHES.READY=1;
        }
        
        if (mem_cycle==MEM_CYCLES)
        {
            if(GetR_W(CURRENT_LATCHES.MICROINSTRUCTION)) /*write*/
            {
                if(GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) /*word*/
                {
                    /*if (CURRENT_LATCHES.MAR&0x0001)
                     {
                     printf("Error: word memory address should be even\n");
                     exit(0);
                     }*/
                    
                    MEMORY[CURRENT_LATCHES.MAR>>1][0]=CURRENT_LATCHES.MDR&0x00FF;
                    MEMORY[CURRENT_LATCHES.MAR>>1][1]=(CURRENT_LATCHES.MDR>>8)&0X00FF;
                }
                
                else /*byte*/
                {
                    if (CURRENT_LATCHES.MAR&0x0001)/*odd memory address*/
                        MEMORY[CURRENT_LATCHES.MAR>>1][1]=(CURRENT_LATCHES.MDR>>8)&0X00FF;
                    else /* even memory address*/
                        MEMORY[CURRENT_LATCHES.MAR>>1][0]=CURRENT_LATCHES.MDR&0x00FF;
                    
                    
                }
            }
            else /*read*/
            {
                if(GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) /*word*/
                {
                    if (CURRENT_LATCHES.MAR&0x0001) /*odd memory address*/
                    {
                        printf("Error: word memory address should be even\n");
                        exit(0);
                    }
                    NEXT_LATCHES.MDR = (MEMORY[CURRENT_LATCHES.MAR>>1][1]<<8) +
                    MEMORY[CURRENT_LATCHES.MAR>>1][0];
                    
                }
                else /*byte*/
                {
                    NEXT_LATCHES.MDR = (MEMORY[CURRENT_LATCHES.MAR>>1][1]<<8) +
                    MEMORY[CURRENT_LATCHES.MAR>>1][0];
                    
                }
                
            }
            mem_cycle=0;
        }
        
    }
    
}



void eval_bus_drivers() {
    
    /*
     * Datapath routine emulating operations before driving the bus.
     * Evaluate the input of tristate drivers
     *             Gate_MARMUX,
     *		 Gate_PC,
     *		 Gate_ALU,
     *		 Gate_SHF,
     *		 Gate_MDR.
     */
    BUS=0;
}


void drive_bus() {
    
    /*
     * Datapath routine for driving the bus from one of the 5 possible
     * tristate drivers.
     */
    
    if (GetGATE_MARMUX(CURRENT_LATCHES.MICROINSTRUCTION)) {
        
        BUS=Low16bits( GetMARMUXvalue());
    }
    if (GetGATE_PC(CURRENT_LATCHES.MICROINSTRUCTION)) {
        
        BUS=Low16bits( GetPCvalue());
    }
    if (GetGATE_ALU(CURRENT_LATCHES.MICROINSTRUCTION)) {
        BUS=Low16bits( GetALUvalue());
    }
    if (GetGATE_SHF(CURRENT_LATCHES.MICROINSTRUCTION)){
        BUS=Low16bits( GetSHFvalue());
    }
    if (GetGATE_MDR(CURRENT_LATCHES.MICROINSTRUCTION)) {
        BUS=Low16bits (GetMDRvalue());
    }
    if (GetGATE_PC_2(CURRENT_LATCHES.MICROINSTRUCTION)) {
        BUS= (CURRENT_LATCHES.PC-2);
    }
    if (GetGATE_PSR(CURRENT_LATCHES.MICROINSTRUCTION)) {
        
        BUS=CURRENT_LATCHES.PSR;
        
    }
    if (GetGATE_Vector(CURRENT_LATCHES.MICROINSTRUCTION)) {
        
        
        BUS = 0x0200+(CURRENT_LATCHES.Vector<<1);
        
    }
    if (GetGATE_SP(CURRENT_LATCHES.MICROINSTRUCTION)) {
        BUS=Low16bits(GetSPvalue());
    }
    if (GetGATE_VA(CURRENT_LATCHES.MICROINSTRUCTION)) {
        BUS=Low16bits(GetVAvalue());
    }
    
    
}


void latch_datapath_values() {
    
    /*
     * Datapath routine for computing all functions that need to latch
     * values in the data path at the end of this cycle.  Some values
     * require sourcing the bus; therefore, this routine has to come
     * after drive_bus.
     */
    if(GetLD_MAR(CURRENT_LATCHES.MICROINSTRUCTION)){
        
        NEXT_LATCHES.MAR=BUS;
    }
    if (GetLD_MDR(CURRENT_LATCHES.MICROINSTRUCTION)){
        if (!GetMIO_EN(CURRENT_LATCHES.MICROINSTRUCTION))
        {
            if (GetDATA_SIZE(CURRENT_LATCHES.MICROINSTRUCTION)) {
                NEXT_LATCHES.MDR=BUS;
            }
            else if(CURRENT_LATCHES.MAR&0x0001)
                NEXT_LATCHES.MDR=((BUS&0x00FF)<<8);
            else NEXT_LATCHES.MDR=(BUS&0x00FF);
            
            if (GetMDRMUX(CURRENT_LATCHES.MICROINSTRUCTION)==1) {
                NEXT_LATCHES.MDR=(CURRENT_LATCHES.MDR|0x0001);
            }
            if (GetMDRMUX(CURRENT_LATCHES.MICROINSTRUCTION)==2) {
                NEXT_LATCHES.MDR=(CURRENT_LATCHES.MDR|0x0002);
            }
            
        }
        
    }
    if (GetLD_PC(CURRENT_LATCHES.MICROINSTRUCTION)) {
        if (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION)==0) {
            
            NEXT_LATCHES.PC = CURRENT_LATCHES.PC + 2;
            
            
        }
        if  (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION)==1){
            NEXT_LATCHES.PC = BUS;
        }
        if (GetPCMUX(CURRENT_LATCHES.MICROINSTRUCTION)==2){
            NEXT_LATCHES.PC = Low16bits (GetADDR2value()+GetADDR1value());
        }
    }
    if (GetLD_REG(CURRENT_LATCHES.MICROINSTRUCTION))
        NEXT_LATCHES.REGS[GetDR(CURRENT_LATCHES.IR)]=BUS;
    
    if (GetLD_IR(CURRENT_LATCHES.MICROINSTRUCTION)) {
        NEXT_LATCHES.IR=BUS;
        
    }
    if (GetLD_CC(CURRENT_LATCHES.MICROINSTRUCTION)){
        
        NEXT_LATCHES.N = 0;
        NEXT_LATCHES.Z = 0;
        NEXT_LATCHES.P = 0;
        if (GetPSR1MUX(CURRENT_LATCHES.MICROINSTRUCTION)==0) {
            
            if( BUS==0 ){
                NEXT_LATCHES.Z = 1;
            }else if((BUS&0x8000)==0x8000){
                NEXT_LATCHES.N = 1;
            }else{
                NEXT_LATCHES.P = 1;
            }
            NEXT_LATCHES.PSR=(CURRENT_LATCHES.PSR&0xFFF8)|((NEXT_LATCHES.N<<2)+(NEXT_LATCHES.Z<<1)+(NEXT_LATCHES.P));
            
        }
        else
        {
            NEXT_LATCHES.N=(BUS&0x0004);
            NEXT_LATCHES.Z=(BUS&0x0002);
            NEXT_LATCHES.P=(BUS&0x0001);
        }
        
    }
    if (GetLD_BEN(CURRENT_LATCHES.MICROINSTRUCTION)) {
        
        NEXT_LATCHES.BEN=(((CURRENT_LATCHES.IR&0x0800)>>11)&CURRENT_LATCHES.N)+(((CURRENT_LATCHES.IR&0x0400)>>10)&CURRENT_LATCHES.Z)+(((CURRENT_LATCHES.IR&0x0200)>>9)&CURRENT_LATCHES.P);
        
    }
    
    if (CURRENT_LATCHES.STATE_NUMBER==30)
        
        NEXT_LATCHES.PC=0;
    
    if (GetLD_EXCT(CURRENT_LATCHES.MICROINSTRUCTION)) {
        
        if (GetEXCTMUX(CURRENT_LATCHES.MICROINSTRUCTION)==0) {
            if (BUS&0x0001) {
                NEXT_LATCHES.EXCT=1;
            }
            else {
                NEXT_LATCHES.EXCT=0;
            }
            
        }
        if (GetEXCTMUX(CURRENT_LATCHES.MICROINSTRUCTION)==1) {
            if ((~(CURRENT_LATCHES.MDR&0x0008))&(CURRENT_LATCHES.PSR&0x8000)) {
                NEXT_LATCHES.EXCT=1;
            }
            else {
                NEXT_LATCHES.EXCT=0;
            }
        }
        if (GetEXCTMUX(CURRENT_LATCHES.MICROINSTRUCTION)==2) {
            if (((CURRENT_LATCHES.MDR&0x0004)>>2)==0) {
                
                NEXT_LATCHES.EXCT=1;
            }
            else
                NEXT_LATCHES.EXCT=0;
        }
    }
    
    if (GetLD_PSR15(CURRENT_LATCHES.MICROINSTRUCTION)) {
        
        if(GetPSR2MUX(CURRENT_LATCHES.MICROINSTRUCTION)==0)
            NEXT_LATCHES.PSR = CURRENT_LATCHES.PSR&0x0007;
        else NEXT_LATCHES.PSR=BUS;
    }
    
    if (GetLD_Vector(CURRENT_LATCHES.MICROINSTRUCTION)) {
        if (GetVectorMUX(CURRENT_LATCHES.MICROINSTRUCTION)==0)
        {
            
            NEXT_LATCHES.Vector=0x01;
            /*printf("what happened\n");*/
        }
        if (GetVectorMUX(CURRENT_LATCHES.MICROINSTRUCTION)==1)
        {
            NEXT_LATCHES.Vector=0x02;
            NEXT_LATCHES.EXCV=0x02;
            /*printf("right");*/
        }
        if (GetVectorMUX(CURRENT_LATCHES.MICROINSTRUCTION)==2)
        {
            NEXT_LATCHES.Vector=0x03;
            NEXT_LATCHES.EXCV=0x03;
        }
        if (GetVectorMUX(CURRENT_LATCHES.MICROINSTRUCTION)==3)
        {
            NEXT_LATCHES.Vector=0x04;
            NEXT_LATCHES.EXCV=0x04;
        }
        if (GetVectorMUX(CURRENT_LATCHES.MICROINSTRUCTION)==4)
        {
            NEXT_LATCHES.Vector=0x05;
            NEXT_LATCHES.EXCV=0x05;
        }
    }
    
    if (GetLD_SSP(CURRENT_LATCHES.MICROINSTRUCTION)) {
        
        NEXT_LATCHES.SSP=CURRENT_LATCHES.REGS[GetSR1(CURRENT_LATCHES.IR)];
        
    }
    
    if (GetLD_USP(CURRENT_LATCHES.MICROINSTRUCTION)) {
        
        NEXT_LATCHES.USP=CURRENT_LATCHES.REGS[GetSR1(CURRENT_LATCHES.IR)];
    }
    if (GetLD_VA(CURRENT_LATCHES.MICROINSTRUCTION)){
        
        NEXT_LATCHES.VA=CURRENT_LATCHES.MAR;
        
    }
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
}