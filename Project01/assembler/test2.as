        lw      0       1       one     
        lw      0       2       ten     
loop    add     1       1       1       
        beq     1       2       done    
        beq     0       0       loop
done    halt
one     .fill   1
ten     .fill   10
