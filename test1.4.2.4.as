        lw          0       1       n
        lw          0       2       r
        lw          0       4       Caddr        
        jalr        4       7                   
        halt
func    lw      0       6       one   
        sw      5       7       Stack  
        add     5       6       5  
        sw      5       1       Stack  
        add     5       6       5 
        sw      5       2       Stack  
        add     5       6       5  
        lw      0       6       neg1  
        beq     2       0       label 
        beq     1       2       label 
        add     1       6        1 
        lw      0       4        Caddr 
        jalr    4       7         
        sw      5       3       Stack 
        lw      0       6       one 
        add     5       6        5 
        lw      0       6        neg1 
        add     2       6         2 
        lw      0       4        Caddr
        jalr    4       7          
        lw      0       6        neg1 
        add     5       6        5 
        lw      5       6       Stack 
        add     3       6        3   
ret     lw      0       6        neg1
        add     5       6        5 
        lw      5       2       Stack 
        add     5       6        5 
        lw      5       1       Stack 
        add     5       6        5 
        lw      5       7       Stack 
        jalr    7       4          
label   lw      0       3        one  
        beq     0       0        ret 
n       .fill    7
r       .fill    3 
Caddr   .fill   func 
one     .fill    1
neg1    .fill   -1 