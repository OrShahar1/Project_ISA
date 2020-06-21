	.word 1024 66                                   # input to arry in address 1024 
	.word 1025 99                                   # input to arry in address 1025
	.word 1026 3                                    # input to arry in address 1026 
	.word 1027 987                                  # input to arry in address 1027 
	.word 1028 -67                                  # input to arry in address 1028 
	.word 1029 -5                                   # input to arry in address 1029 
	.word 1030 5                                    # input to arry in address 1030 
	.word 1031 487                                  # input to arry in address 1031 
	.word 1032 -995                                 # input to arry in address 1032 
	.word 1033 1033                                 # input to arry in address 1033 
	.word 1034 7                                    # input to arry in address 1034 
	.word 1035 222                                  # input to arry in address 1035 
	.word 1036 11                                   # input to arry in address 1036 
	.word 1037 -11                                  # input to arry in address 1037 
	.word 1038 -12                                  # input to arry in address 1038 
	.word 1039 2055                                 # input to arry in address 1039 
	
	
	add $t1, $zero, $zero, 0                        # $t1 = 0 // i 
	add $a1, $zero, $imm, 1024                      # $a1 = 1024 -  Address of arry //&A
	add $a0, $zero, $imm, 16                        # $a0 = 16 - len of arry //N
	
Loop1:
	add $t0, $a1, $zero, 0                          # $t0 = $a1 = 1024 //j
	
Loop2:
	lw $t2, $t0, $zero, 0                           # $t2 = A[j]
	lw $t3, $t0, $imm, 1                            # $t3 = A[j+1]
	bgt $imm, $t2, $t3, Swap                        # if A[j] > A[j+1] jump to Swap
	
Again:
	add $t0, $t0, $imm, 1                           # $t0 = $t0 + 1
	sub $t2, $a0, $imm, 0                           # $t2 = $a0 - 1
	sub $t3, $t2, $t1, 0                            # $t3 = $a0 - 1 - $t1 // 16 - i - 1 
	add $t3, $t3, $a1, 0                            # $t3 = $a1 + $t3     // 16 - i - 1 + 1024
Loop_1_2:
	blt $imm, $t0, $t3, Loop2                       # if $t0 < $t3 Jump to Loop2    // 1024 + j < 16 - i - 1 + 1024
	add $t1, $t1, $imm, 1                           # $t1 = $t1 + 1
	blt $imm, $t1, $t2, Loop1                       # if $t1 < $t2 
	jal $imm, $zero, $zero, Exit                    # jump to Exit
	
Swap:
	sw $t2, $imm, $t0, 1                            # A[j+1] = A[j]
	sw $t3, $zero, $t0, 0                           # A[j] = A[j+1]
	beq $imm, $zero, $zero, Again                   # jump to Again
	
Exit:
	halt $zero, $zero, $zero, 0                     # halt

	
	