        lw      0       2      mcand   
        lw      0       3      mplier  
        lw      0       4      fift   
        lw      0       5      mask   
        lw      0       6      incre  
        nor     3       3       3     
loop    beq     4       6      end   
        nor     5       5       7     
        nor     7       3       7     
        beq     7       0      skip   
        add     1       2       1
skip    noop
        lw      0       7       one
        add     2       2        2 
        add     5       5        5 
        add     6       7        6
        beq     0       0       loop
end     halt
mcand   .fill    1103 
mplier  .fill    7043                   
fift    .fill    15
mask    .fill    1
incre   .fill    0
one     .fill    1