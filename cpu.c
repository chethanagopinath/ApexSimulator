/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define IS_DISPLAY 1

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  /*
  // ptr ==> Starting address of memory to be filled
  // x   ==> Value to be filled
  // n   ==> Number of bytes to be filled starting
  //         from ptr to be filled
  void *memset(void *ptr, int x, size_t n);
  */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (IS_DISPLAY) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "LOAD") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "LDR") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "STR") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rs1, stage->rs2, stage->rs3);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }

  if (strcmp(stage->opcode, "ADD") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "ADDL") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "SUB") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "SUBL") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }

  if (strcmp(stage->opcode, "MUL") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "AND") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "OR") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }

  if (strcmp(stage->opcode, "EXOR") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];
  if (!stage->busy && !stage->stalled) {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->rs3 = current_ins->rs3;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    //printf("\nPrinting opcode :  %s\n", stage->opcode );



    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    cpu->stage[DRF] = cpu->stage[F];

    if (IS_DISPLAY) {
      print_stage_content("Fetch", stage);
    }
  }
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];

  if (!stage->busy && !stage->stalled) {

    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0)
    {
      stage->rs1_value = cpu->regs[stage->rs1];
      stage->rs2_value = cpu->regs[stage->rs2];
    }

    if(strcmp(stage->opcode, "LOAD") == 0)
    {
        stage->rs1_value = cpu->regs[stage->rs1];
    }

    if(strcmp(stage->opcode, "LDR") == 0)
    {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
    }

    if(strcmp(stage->opcode, "STR") == 0)
    {
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        stage->rs3_value = cpu->regs[stage->rs3];
    }

    if (strcmp(stage->opcode, "ADD") == 0)
    {
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
      //printf("\nrs1 and rs2 values: %d %d", stage->rs1_value, stage->rs2_value);
    }

    if (strcmp(stage->opcode, "ADDL") == 0)
    {
      stage->rs1_value = cpu->regs[stage->rs1];
      //printf("%d\n",stage->rs1_value);
    }

    if (strcmp(stage->opcode, "SUB") == 0)
    {
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
      //printf("\nrs1 and rs2 values: %d %d", stage->rs1_value, stage->rs2_value);
    }

    if (strcmp(stage->opcode, "SUBL") == 0)
    {
      stage->rs1_value = cpu->regs[stage->rs1];
    }

    if (strcmp(stage->opcode, "MUL") == 0)
    {
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
    }
    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
        //printf("Yes\n");
        //printf("%s\n", stage->opcode);
    }

    if (strcmp(stage->opcode, "AND") == 0)
    {
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
    }

    if (strcmp(stage->opcode, "OR") == 0)
    {
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
    }

    if (strcmp(stage->opcode, "EXOR") == 0)
    {
        stage->rs1_value= cpu->regs[stage->rs1];
        stage->rs2_value= cpu->regs[stage->rs2];
    }




    /* Copy data from decode latch to execute latch*/
    cpu->stage[EX1] = cpu->stage[DRF];

    if (IS_DISPLAY) {
      print_stage_content("Decode/RF", stage);
    }
  }
  return 0;
}

/*
 *  Execute1 Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
execute1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX1];
  if (!stage->busy && !stage->stalled) {
    //printf("\nExec1\n");

    /* STORE */
    if (strcmp(stage->opcode, "STORE") == 0) {
      stage->mem_address = stage->rs2_value + stage->imm;
    }

    /* LOAD */
    if (strcmp(stage->opcode, "LOAD") == 0) {
      stage->mem_address = stage->rs1_value + stage->imm;
    }

    /* LDR */
    if (strcmp(stage->opcode, "LDR") == 0) {
      stage->mem_address = stage->rs1_value + stage->rs2_value;
    }

    /* STR */
    if (strcmp(stage->opcode, "STR") == 0) {
      stage->mem_address = stage->rs2_value + stage->rs3_value;
    }

    //printf("%s\n",stage->opcode );

    /* ADD */
    if (strcmp(stage->opcode, "ADD") == 0) {
        //printf("\nOpcode is ADD\n");
        //printf("\n%d, %d\n", stage->rs1_value, stage->rs2_value);
        stage->buffer = stage->rs1_value + stage->rs2_value;
        //printf("\nStage->buffer : %d\n", stage->buffer );
    }

    /* SUB */
    if (strcmp(stage->opcode, "SUB") == 0) {
        stage->buffer = stage->rs1_value - stage->rs2_value;
    }

    /* SUBL */
    if (strcmp(stage->opcode, "SUBL") == 0) {
      stage->buffer = stage->rs1_value - stage->imm;
    }

    /* ADDL */
    if (strcmp(stage->opcode, "ADDL") == 0) {
      stage->buffer = stage->rs1_value + stage->imm;
    }

    /* MUL */
    if (strcmp(stage->opcode, "MUL") == 0) {
        stage->buffer = stage->rs1_value * stage->rs2_value;
      }

    /* AND */
    if (strcmp(stage->opcode, "AND") == 0) {
      stage->buffer = stage->rs2_value & stage->rs1_value;
    }

    /* OR */
    if (strcmp(stage->opcode, "OR") == 0) {
      stage->buffer = stage->rs2_value | stage->rs1_value;
    }

    /* EX-OR */
    if (strcmp(stage->opcode, "EXOR") == 0) {
      printf("\nEXOR %d %d",stage->rs2_value, stage->rs1_value);
      stage->buffer = stage->rs2_value ^ stage->rs1_value;
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      stage->buffer = stage->imm;
    }

    /* Copy data from Execute1 latch to Execute2 latch*/
    cpu->stage[EX2] = cpu->stage[EX1];
    //printf("\nBuffer: %d", stage->buffer);

    if (IS_DISPLAY) {
      print_stage_content("Execute1", stage);
    }
  }
  return 0;
}

/*
 *  Execute2 Stage of APEX Pipeline
 *
 */

int
execute2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX2];
  if (!stage->busy && !stage->stalled) {

    /* Copy data from Execute2 latch to Memory1 latch*/
    cpu->stage[MEM1] = cpu->stage[EX2];

    if (IS_DISPLAY) {
      print_stage_content("Execute2", stage);
    }
  }
  return 0;
}


/*
 *  Memory1 Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
memory1(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM1];
  if (!stage->busy && !stage->stalled) {

    /* STORE */
    if (strcmp(stage->opcode, "STORE") == 0) {
      cpu->data_memory[stage->mem_address] = stage->rs1_value;
    }

    /* STR */
    if (strcmp(stage->opcode, "STR") == 0) {
      cpu->data_memory[stage->mem_address] = stage->rs1_value;
    }

    /* LOAD */
    if (strcmp(stage->opcode, "LOAD") == 0) {
      stage->buffer= cpu->data_memory[stage->mem_address];
    }

    /* LDR */
    if (strcmp(stage->opcode, "LDR") == 0) {
      stage->buffer= cpu->data_memory[stage->mem_address];
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    /* Copy data from Memory1 latch to Memory2 latch*/
    cpu->stage[MEM2] = cpu->stage[MEM1];

    if (IS_DISPLAY) {
      print_stage_content("Memory1", stage);
    }
  }
  return 0;
}

/*
 *  Memory2 Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
memory2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM2];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
    }


    /* Copy data from memory2 latch to writeback latch*/
    cpu->stage[WB] = cpu->stage[MEM2];

    if (IS_DISPLAY) {
      print_stage_content("Memory2", stage);
    }
  }
  return 0;
}



/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled) {

    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "ADD") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "ADDL") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "SUB") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "SUBL") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if(strcmp(stage->opcode, "MUL") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "AND") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "OR") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "EXOR") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    if (strcmp(stage->opcode, "LDR") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    cpu->ins_completed++;

    if (IS_DISPLAY) {
      print_stage_content("Writeback", stage);
    }
  }
  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
 int
 APEX_cpu_run(APEX_CPU* cpu)
 {
   while (1) {

     /* All the instructions committed, so exit */
     if (cpu->ins_completed == cpu->code_memory_size) {
       printf("(apex) >> Simulation Complete");
       for(int i=0;i<15;i++){
       printf("REG[%d]: %d\n",i,cpu->regs[i]);
     }

       break;
     }

     if (IS_DISPLAY) {
       printf("--------------------------------\n");
       printf("Clock Cycle #: %d\n", cpu->clock);
       printf("--------------------------------\n");
     }

     writeback(cpu);
     memory2(cpu);
     memory1(cpu);
     execute2(cpu);
     execute1(cpu);
     decode(cpu);
     fetch(cpu);

     /*
     fetch(cpu);
     decode(cpu);
     execute1(cpu);
     execute2(cpu);
     memory1(cpu);
     memory2(cpu);
     writeback(cpu);
     */
     cpu->clock++;
   }

   return 0;
 }
