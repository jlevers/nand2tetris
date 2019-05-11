// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Mult.asm

// Multiplies R0 and R1 and stores the result in R2.
// (R0, R1, R2 refer to RAM[0], RAM[1], and RAM[2], respectively.)

// Put your code here.
// Initialize sum to 0
  @R2
  M=0
(LOOP)
  @R1
  D=M
  @END
  D;JLE  // Jump to @END if D <= 0
  @R2
  D=M    // Set D to sum so far
  @R0
  D=D+M  // Add R0 to D
  @R1
  M=M-1  // Decrement R1
  @R2
  M=D    // Set sum to D
  @R1
  D=M    // Set D to number of iterations left
  @LOOP
  0;JMP  // Keep looping
(END)
  @END
  0;JMP
