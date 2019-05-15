// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed.
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

  @jmp_to
  M=0

  @24575   // RAM address of last pixel value (16384 + 8192 - 1)
  D=A
  @screen_end
  M=D

(STATE)
  @KBD
  D=M

  @BLACK
  D;JNE
  @WHITE
  D;JEQ
  @STATE
  0;JMP

(BLACK)
  D=-1
  @DRAW
  0;JMP

(WHITE)
  D=0
  @DRAW
  0;JMP

(DRAW)
  @val
  M=D
  @SCREEN
  D=A
  @i
  M=D

  (LOOP)
    @val
    D=M
    @i
    A=M
    M=D
    @i
    D=M
    @screen_end
    D=M-D
    @i
    M=M+1
    @LOOP
    D;JGT

  @STATE
  0;JMP

(END)
  @END
  0;JMP
