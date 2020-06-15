add $t2, $zero, $imm, 1						  # $t2 = 1
out $t2, $zero, $imm, 0						  # enable irq0
add $t3, $zero, $imm, 255                     # $t3 = 255
out $t3, $zero, $imm, 13                      # timermax = 255 , frequency = 256HZ
out $t2, $zero, $imm, 11                      # timerenable=1
add $t0, $zero, $imm, 6						  # set $t0 = 6
out $t2, $zero, $imm, 9                       # first led on led[0]=1
out $imm, $t0, $zero, Int_handler			  # irqhandler=Int_handler  
 
Loop :
    in $s1, $zero, $imm, 9                        # $s1=leds
	beq $imm, $s1, $zero, Exit                    # stay in loop until leds=0
	beq $imm, $zero, $zero, Loop				  # back to Loop

	
Int_handler:
	sll $t0, $s1, $imm, 1                         # $to=$s1<<1 move 1 left
	out $t0, $zero, $imm, 9                       # led=$t0
	add $t1, $zero, $zero, 0					  # $t1=0
	out $t1, $zero, $imm, 3						  # irqstatus=0
	reti $zero, $zero, $zero, 0					  # return from interrupt
	
Exit:
	halt $zero, $zero, $zero, 0
	