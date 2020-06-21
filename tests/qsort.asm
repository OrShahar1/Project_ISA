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

Sphandler:
	add $sp, $zero, $imm, 1000   			        # $sp = 1000
	add $a1, $zero, $imm, 0                         # $a1 = 0
	add $a0, $zero, $imm, 15                        # $a0 = 15
	add $s0, $zero, $imm, 1024                      # $s0 = 1024
	
Start: 
	jal $imm, $zero, $zero, Quicksort               # Quicksort
	jal $imm, $zero, $zero, Exit                    # Exit

Quicksort:
	add $sp, $sp, $imm, -3                          # make room for 3 itmes
	sw $a1, $sp, $imm, 2                            # p 
	sw $a0, $sp, $imm, 1                            # r
	sw $ra, $sp, $imm, 0                            # return address
	blt $imm, $a1, $a0, PartitionBegin              # if p < r -> PartitionBegin
	beq $imm, $zero, $zero, END                     # END
	
PartitionBegin:
	add $t0, $a1, $imm, -1                          # $t0 = $a1-1
	add $t3, $a0, $imm, 1                           # $t3 = $a0+1
	
Loop1:
	lw $t1, $s0, $a1, 0                             # $t1 = pivot
	add $t3, $t3, $imm, -1                          # $t3 = $t3 - 1
	lw $t2, $t3, $s0, 0                             # $t2=A[j]
	bgt $imm, $t2, $t1, Loop1                       # Loop1 while A[j] <= pivot
	
Loop2:
	add $t0, $t0, $imm, 1                           # $t0 = $t0 + 1
	lw $t2, $s0, $t0, 0                             # $t1 = A[i]
	blt $imm, $t2, $t1, Loop2                       # A[j] >= pivot -> Loop2 
	blt $imm, $t0, $t3, HelperSwap                  # swap if smaller
	add $a0, $t3, $imm, 0                           # $a0 = $t3 = j
	jal $imm, $zero, $zero, Quicksort               # Quicksort
	lw $a0, $sp, $imm, 1                            # restore $a0
	add $a1, $t3, $imm, 1                           # $a1 = $t3 + 1
	jal $imm, $zero, $zero, Quicksort               # Quicksort

END:
	lw $ra, $sp, $imm, 0 						    # restore $ra
	lw $a0, $sp, $imm, 1						    # restore $a0
	lw $a1, $sp, $imm, 2					        # restore $a1
	add $sp, $sp, $imm, 3                           # close room of 3 itmes
	beq $ra, $zero, $zero, 0                        # return
	
HelperSwap:
	lw $t1, $t3, $s0, 0                             # $t1 = A[j]
	sw $t2, $t3, $s0, 0                             # A[j] = A[i]
	sw $t1, $t0, $s0, 0                             # A[i] = A[j]
	beq $imm, $zero, $zero, Loop1                   # Loop1 
		
Exit:
	halt $zero, $zero, $zero, 0                     # halt