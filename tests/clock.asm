
	jal $imm, $zero, $zero, clock
	jal $imm, $zero, $zero, Exit

clock:
	add $t3, $zero, $imm, 255                                       # $t3 = 255
	out $t3, $zero, $imm, 13 									    # timermax = 255 , frequency = 256HZ
	add $t1, $zero, $imm, 1                                         # $t1 = 1 
	lw $t0, $zero, $imm, 1024                                       # load MEM[1024] = 195955
	out $t0, $zero, $imm, 10                                        # display the time
	out $t1, $zero, $imm, 11                                        # timer on - starts counting 	
	
Loop: 
	in $t1, $zero, $imm, 3                                          # take value of irq0status
	beq $imm, $zero, $t1, Loop                                      # if irq0status == 0 stay in Loop . else 

Display:
	out $zero, $zero, $imm, 3                                       # timer off - stop counting 
	add $t0, $t0, $imm, 1                                           # $t0= $t0 + 1 
	out $t0, $zero, $imm, 10                                        # display the time

Check_time:
	add $t3, $zero, $imm, 9                                         # $t3 = 9 
	and $t2, $imm, $t0, 0xf                                         # sampling the 4 LSB
	beq $imm, $t2, $t3, CriticTime                                  # if 195959 then jump to CriticTime
	add $t3, $zero, $imm, 5                                         # $t3 = 5 
	and $t2, $t0, $imm, 0xf                                         # sampling the 4 LSB
	beq $ra, $t2, $t3, 0                                            # if time = 200005 then jump to Exit
	beq $imm, $zero, $zero, Loop                                    # Loop again , Pass to checks


CriticTime:
	lw $t0, $zero, $imm, 1025                                       # load MEM[1025] = 200000
	beq $imm, $zero, $zero, Loop                                    # Loop again , with 2000000

Exit: 
	halt $zero, $zero, $zero, 0				                        # exit simulator halt ececution






	.word 1024 0X195955                                           # start time 
	.word 1025 0x1fffff                                           # time before 20:00:00 , critic time
	
	


