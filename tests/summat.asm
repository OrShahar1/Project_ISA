	.word 0x100 7                                    # input to mat1 
	.word 0x101 6                                    # input to mat1 
	.word 0x102 6                                    # input to mat1 
	.word 0x103 6                                    # input to mat1 
	.word 0x104 2                                    # input to mat1 
	.word 0x105 1                                    # input to mat1 
	.word 0x106 2                                    # input to mat1 
	.word 0x107 9                                    # input to mat1 
	.word 0x108 66                                   # input to mat1 
	.word 0x109 45                                   # input to mat1 
	.word 0x10a 66                                   # input to mat1 
	.word 0x10b 66                                   # input to mat1 
	.word 0x10c 66                                   # input to mat1 
	.word 0x10d 66                                   # input to mat1 
	.word 0x10e 66                                   # input to mat1 
	.word 0x10f 0                                    # input to mat1 

	.word 0x110 2                                    # input to mat2
	.word 0x111 66                                   # input to mat2 	
	.word 0x112 61                                   # input to mat2 
	.word 0x113 62                                   # input to mat2 
	.word 0x114 66                                   # input to mat2 
	.word 0x115 66                                   # input to mat2 
	.word 0x116 66                                   # input to mat2 
	.word 0x117 66                                   # input to mat2 
	.word 0x118 66                                   # input to mat2 
	.word 0x119 65                                   # input to mat2 
	.word 0x11a 66                                   # input to mat2 
	.word 0x11b 66                                   # input to mat2 
	.word 0x11c 69                                   # input to mat2 
	.word 0x11d 66                                   # input to mat2 
	.word 0x11e 66                                   # input to mat2 
	.word 0x11f 3                                    # input to mat2 
		
	
	jal $imm, $zero, $imm, Summat                    # Jump to Summat
	jal $imm, $zero, $imm, Exit                      # Jump to Exit
Summat:
	add $t1, $imm, $zero, 0x100                      # $t1 = 0x100
For:
	lw $t0, $t1, $zero, 0                            # $t0 = MEM[$t1]
	lw $t3, $t1, $imm, 16                            # $t3 = MEM[$t1 + 16]
	add $t2, $t3, $t0, 0                             # $t2 = MEM[$t1 + 16] + MEM[$t1] = $t0 + $t1 
	sw $t2, $t1, $imm, 32                            # Mem[$t1+32] = $t2
	
Next_iter:
	add $t1, $t1, $imm, 1                            # $t1 = $t1 + 1
	beq $ra, $t1, $imm, 0x110                        # Exit ? 
	beq $imm, $zero, $zero, For                      # Jump to For

Exit:
	halt $zero, $zero, $zero, 0                      # halt


