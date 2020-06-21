	jal $imm, $zero, $zero, disktest
	jal $imm, $zero, $zero, Exit

disktest:	
	add $t3, $zero,$imm, 1024                        # $t3 = 1024
	out $t3, $zero, $imm ,16                         # diskbuffer begin in MEM[1024] . SIZE of the buffer is 128 lines in MEM
	add $t1, $zero,$imm, 0  						 # $t1 = 0
	add $t0, $zero,$imm, 1                           # $t0 = 1 

READorWriteOperetion:
	out $t1, $zero, $imm, 15                         # set sector number $to
	out $t0, $zero, $imm, 14                         # dick cmd READ only 
	beq $imm, $zero, $zero, WaitLoop                 # waiting for sidkstaus to be not busy 
	
ReadtoWrite:
	add $t2, $zero,$imm, 1                           # $t2 =1 
	bne $imm, $t2 ,$t0 , WritetoRead                 # if $t0 != 1 goto WritetoRead else stay
	add $t1, $t1 ,$imm, 4                            # go to the next sector (wirte sector !!) - example 0->4->1->5...
	add $t0, $zero, $imm, 2                          # $t0 = 2   WRITE MODE
	beq $imm, $zero, $zero, READorWriteOperetion     # READorWriteOperetion again
			
WritetoRead: 
	add $t0, $imm, $zero, 1                          # set $t0 to READ MODE
	sub $t1, $t1, $imm, 3                            # go to the next sector (read sector !!) - example 0->4->1->5...
	beq $ra, $imm, $t1, 4                            # if in sector 4 go to exit
	beq $imm, $zero, $zero, READorWriteOperetion     # go to READorWriteOperetion
	
WaitLoop:
	in $t3, $zero, $imm , 17                         # Get diskstatus
	bne $imm, $t3, $zero, WaitLoop                   # if busy go to WaitLoop
	beq $imm, $zero, $zero, ReadtoWrite              # jump to  ReadtoWrite
	
Exit:
	halt $zero, $zero, $zero, 0				         # exit simulator halt ececution






















