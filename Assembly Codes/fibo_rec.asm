addi x3, x0, 10
addi x8, x0, 1
jal x1,fib 
beq x0,x0,exit 
fib:blt x8,x3,l1 
    addi x4,x3,0
    jalr x0, 0(x1) 
l1: addi x2,x2,-12
    sw x1,0(x2)
    sw x3,4(x2)
    addi x3,x3,-1
    jal x1,fib 
    lw x3,4(x2)
    sw x4,8(x2)
    addi x3,x3,-2
    jal x1,fib 
    lw x5,8(x2)
    add x4,x4,x5
    lw x1,0(x2)
    addi x2,x2,12
    jalr x0, 0(x1) 
exit:
