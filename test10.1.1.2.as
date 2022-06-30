        lw      0       1       ArrayS
        lw      0       2       Num
        lw      0       3       Fcall
        jalr    3       7
        halt
Find    lw      0       6       NegOne
        add     0       0        5
Top     add     2       6        2
        add     1       2        4
        lw      4       3        0
        beq     3       0        skip
        lw      0       4         One
        add     5       4         5
skip    beq     0       2        Done
        beq     0       0        Top
Done    jalr    7       3       
zero    .fill  0
NegOne  .fill -1
One     .fill  1
Fcall   .fill  Find 
ArrayS  .fill  Array
Num     .fill  9
Array   .fill  0
        .fill  3
        .fill  1
        .fill  -1
        .fill   0
        .fill   -1
        .fill   0
        .fill   1
        .fill   -2