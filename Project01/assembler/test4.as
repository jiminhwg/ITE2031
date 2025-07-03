        lw      0       1       three    
        lw      0       2       five     
start   add     1       2       1        reg1 = reg1 + reg2
        beq     1       2       equal    
        beq     0       0       start    
equal   halt                             
three   .fill   3
five    .fill   5
