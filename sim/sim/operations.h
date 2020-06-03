#ifndef  OPERATIONS_H
#define OPERATIONS_H

#include "cpu.h"

void add_op(struct CPU *cpu);
void sub_op(struct CPU *cpu);
void and_op(struct CPU *cpu);
void or_op(struct CPU *cpu);
void sll_op(struct CPU *cpu);
void sra_op(struct CPU *cpu);
void srl_op(struct CPU *cpu);
void beq_op(struct CPU *cpu);
void beq_op(struct CPU *cpu);
void bne_op(struct CPU *cpu);
void blt_op(struct CPU *cpu);
void bgt_op(struct CPU *cpu);
void ble_op(struct CPU *cpu);
void bge_op(struct CPU *cpu);
void jal_op(struct CPU *cpu);
void lw_op(struct CPU *cpu);
void sw_op(struct CPU *cpu);
void reti_op(struct CPU *cpu);
void in_op(struct CPU *cpu);
void out_op(struct CPU *cpu);
void halt_op(struct CPU *cpu);

#endif // ! OPERATIONS_H

