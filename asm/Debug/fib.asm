	add $sp, $zero, $imm, 1		# set $sp = 1
	out $sp, $zero, $imm, 2		# enable irq2
	sll $sp, $sp, $imm, 11		# set $sp = 1 << 11 = 2048
	add $t0, $zero, $imm, 6		# set $t0 = 6
	out $imm, $t0, $zero, L3	# set irqhandler as L3
	lw $a0, $zero, $imm, 1024	# get x from address 1024
	jal $imm, $zero, $zero, fib	# calc $v0 = fib(x)
	sw $v0, $zero, $imm, 1025	# store fib(x) in 1025
	halt $zero, $zero, $zero, 0	# halt
fib:
	add $sp, $sp, $imm, -3		# adjust stack for 3 items
	sw $s0, $sp, $imm, 2		# save $s0
	sw $ra, $sp, $imm, 1		# save return address
	sw $a0, $sp, $imm, 0		# save argument
	add $t0, $zero, $imm, 1		# $t0 = 1
	bgt $imm, $a0, $t0, L1		# jump to L1 if x > 1
	add $v0, $a0, $zero, 0		# otherwise, fib(x) = x, copy input
	beq $imm, $zero, $zero, L2	# jump to L2
L1:
	sub $a0, $a0, $imm, 1		# calculate x - 1
	jal $imm, $zero, $zero, fib	# calc $v0=fib(x-1)
	add $s0, $v0, $imm, 0		# $s0 = fib(x-1)
	lw $a0, $sp, $imm, 0		# restore $a0 = x
	sub $a0, $a0, $imm, 2		# calculate x - 2
	jal $imm, $zero, $zero, fib	# calc fib(x-2)
	add $v0, $v0, $s0, 0		# $v0 = fib(x-2) + fib(x-1)
	lw $a0, $sp, $imm, 0		# restore $a0
	lw $ra, $sp, $imm, 1		# restore $ra
	lw $s0, $sp, $imm, 2		# restore $s0
L2:
	add $sp, $sp, $imm, 3		# pop 3 items from stack
	add $t0, $a0, $zero, 0		# $t0 = $a0
	sll $t0, $t0, $imm, 16		# $t0 = $t0 << 16
	add $t0, $t0, $v0, 0		# $t0 = $t0 + $v0
	out $t0, $zero, $imm, 10	# write $t0 to display
	beq $ra, $zero, $zero, 0	# and return
L3:
	in $t1, $zero, $imm, 9		# read leds register into $t1
	sll $t1, $t1, $imm, 1		# left shift led pattern to the left
	or $t1, $t1, $imm, 1		# lit up the rightmost led
	out $t1, $zero, $imm, 9		# write the new led pattern
	out $zero, $zero, $imm, 5	# clear irq2 status
	reti $zero, $zero, $zero, 0	# return from interrupt
	.word 1024 7