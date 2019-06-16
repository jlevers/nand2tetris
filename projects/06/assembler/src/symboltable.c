/*
 * Symbol table generator for the nand2tetris assembler.
 * @author Jesse Evers
 * @email jesse27999@gmail.com
 * @created 6/3/19
 */

#include "../hashtable/src/hash_table.h"
#include "symboltable.h"

ht_hash_table* constructor() {
    ht_hash_table *ht = ht_new();
    ht_insert(ht, "SP", "0000000000000000");
    ht_insert(ht, "LCL", "0000000000000001");
    ht_insert(ht, "ARG", "0000000000000010");
    ht_insert(ht, "THIS", "0000000000000011");
    ht_insert(ht, "THAT", "0000000000000100");
    ht_insert(ht, "R0", "0000000000000000");
    ht_insert(ht, "R1", "0000000000000001");
    ht_insert(ht, "R2", "0000000000000010");
    ht_insert(ht, "R3", "0000000000000011");
    ht_insert(ht, "R4", "0000000000000100");
    ht_insert(ht, "R5", "0000000000000101");
    ht_insert(ht, "R6", "0000000000000110");
    ht_insert(ht, "R7", "0000000000000111");
    ht_insert(ht, "R8", "0000000000001000");
    ht_insert(ht, "R9", "0000000000001001");
    ht_insert(ht, "R10", "0000000000001010");
    ht_insert(ht, "R11", "0000000000001011");
    ht_insert(ht, "R12", "0000000000001100");
    ht_insert(ht, "R13", "0000000000001101");
    ht_insert(ht, "R14", "0000000000001110");
    ht_insert(ht, "R15", "0000000000001111");
    ht_insert(ht, "SCREEN", "0100000000000000");
    ht_insert(ht, "KBD", "0110000000000000");
    return ht;
}
