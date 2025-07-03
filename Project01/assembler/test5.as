        lw      0       1       eight   
        lw      0       2       ten     
start   add     1       2       1       
        beq     1       2       equal    
        blt     1       2       less    
        beq     0       0       start    
equal   halt                            
less    add     1       1       1      
        beq     0       0       start   
eight   .fill   8
ten     .fill   10
