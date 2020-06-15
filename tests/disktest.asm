	jal $imm, $zero, $zero, disktest
	jal $imm, $zero, $zero, Exit

disktest:	
	add $t2, $zero,$imm, 1024                        # $t2 = 1024
	out $t2, $zero, $imm ,16                         # diskbuffer begin in MEM[1024] . SIZE of the buffer is 128 lines in MEM
	add $t0, $zero,$imm, 0  						 # $t0 = 0
	add $t1, $zero,$imm, 1                           # $t1 = 1 

READorWriteOperetion:
	out $t0, $zero, $imm, 15                        # set sector number $to
	out $t1, $zero, $imm, 14                        # dick cmd READ only 
	beq $imm, $zero, $zero, WaitLoop                 # waiting for sidkstaus to be not busy 
	
ReadtoWrite:
	add $t3, $zero,$imm, 1                           # $t3 =1 
	bne $imm, $t3 ,$t1 , WritetoRead                 # if $t1 != 1 goto WritetoRead else stay
	add $t0, $t0 ,$imm, 4                            # go to the next sector (wirte sector !!) - example 0->4->1->5...
	add $t1, $zero, $imm, 2                          # $t1 = 2   WRITE MODE
	beq $imm, $zero, $zero, READorWriteOperetion     # READorWriteOperetion again
			
WritetoRead: 
	add $t1, $imm, $zero, 1                          # set $t1 to READ MODE
	sub $t0, $t0, $imm, 3                            # go to the next sector (read sector !!) - example 0->4->1->5...
	beq $ra, $imm, $t0, 4                            # if in sector 4 go to exit
	beq $imm, $zero, $zero, READorWriteOperetion     # go to READorWriteOperetion
	
WaitLoop:
	in $t2, $zero, $imm , 17                         # Get diskstatus
	bne $imm, $t2, $zero, WaitLoop                   # if busy go to WaitLoop
	out $zero, $zero, $imm , 1                       # turn off irq1
	beq $imm, $zero, $zero, ReadtoWrite              # jump to  ReadtoWrite
	
Exit:
	halt $zero, $zero, $zero, 0				         # exit simulator halt ececution






















