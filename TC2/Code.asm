0:       LW R1, R0, #0       // Load Mem[R0+0] into R1 - Value 8
4:  C1:  ADDI R2, R2, #4     // Increment R2 by 4
8:       BNE R1, R2, R1      // If R1 != R2 branch to C1
12:      HALT                // Halt

/* Binary
00000000000000000000000010000011
00000000010000010000000100010011
11111110000100010001111011100011
11111111111111111111111111111111
*/