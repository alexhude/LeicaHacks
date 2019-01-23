
#include "fr.hpp"

instruc_t Instructions[] = {
    { "",           0                               },  // null instruction
    { "ADD",        CF_USE1|CF_USE2|CF_CHG2         },  // add word data of source register / 4-bit immediate data to destination register
    { "ADD2",       CF_USE1|CF_USE2|CF_CHG2         },  // add 4-bit immediate data to destination register
    { "ADDC",       CF_USE1|CF_USE2|CF_CHG2         },  // add word data of source register and carry bit to destination register
    { "ADDN",       CF_USE1|CF_USE2|CF_CHG2         },  // add word data of source register / immediate data to destination register
    { "ADDN2",      CF_USE1|CF_USE2|CF_CHG2         },  // add immediate data to destination register
    { "SUB",        CF_USE1|CF_USE2|CF_CHG2         },  // subtract word data in source register from destination register
    { "SUBC",       CF_USE1|CF_USE2|CF_CHG2         },  // subtract word data in source register and carry bit from destination register
    { "SUBN",       CF_USE1|CF_USE2|CF_CHG2         },  // subtract word data in source register from destination register
    { "CMP",        CF_USE1|CF_USE2                 },  // compare word / immediate data in source register and destination register
    { "CMP2",       CF_USE1|CF_USE2                 },  // compare immediate data and destination register
    { "AND",        CF_USE1|CF_USE2|CF_CHG2         },  // and word data of source register to destination register / data in memory
    { "ANDH",       CF_USE1|CF_USE2|CF_CHG2         },  // and half-word data of source register to data in memory
    { "ANDB",       CF_USE1|CF_USE2|CF_CHG2         },  // and byte data of source register to data in memory
    { "OR",         CF_USE1|CF_USE2|CF_CHG2         },  // or word data of source register to destination register / data in memory
    { "ORH",        CF_USE1|CF_USE2|CF_CHG2         },  // or half-word data of source register to data in memory
    { "ORB",        CF_USE1|CF_USE2|CF_CHG2         },  // or byte data of source register to data in memory
    { "EOR",        CF_USE1|CF_USE2|CF_CHG2         },  // exclusive or word data of source register to destination register / data in memory
    { "EORH",       CF_USE1|CF_USE2|CF_CHG2         },  // exclusive or half-word data of source register to data in memory
    { "EORB",       CF_USE1|CF_USE2|CF_CHG2         },  // exclusive or byte data of source register to data in memory
    { "BANDL",      CF_USE1|CF_USE2|CF_CHG2         },  // and 4-bit immediate data to lower 4 bits of byte data in memory
    { "BANDH",      CF_USE1|CF_USE2|CF_CHG2         },  // and 4-bit immediate data to higher 4 bits of byte data in memory
    { "BORL",       CF_USE1|CF_USE2|CF_CHG2         },  // or 4-bit immediate data to lower 4 bits of byte data in memory
    { "BORH",       CF_USE1|CF_USE2|CF_CHG2         },  // or 4-bit immediate data to higher 4 bits of byte data in memory
    { "BEORL",      CF_USE1|CF_USE2|CF_CHG2         },  // eor 4-bit immediate data to lower 4 bits of byte data in memory
    { "BEORH",      CF_USE1|CF_USE2|CF_CHG2         },  // eor 4-bit immediate data to higher 4 bits of byte data in memory
    { "BTSTL",      CF_USE1|CF_USE2                 },  // test lower 4 bits of byte data in memory
    { "BTSTH",      CF_USE1|CF_USE2                 },  // test higher 4 bits of byte data in memory
    { "MUL",        CF_USE1|CF_USE2                 },  // multiply word data
    { "MULU",       CF_USE1|CF_USE2                 },  // multiply unsigned word data
    { "MULH",       CF_USE1|CF_USE2                 },  // multiply half-word data
    { "MULUH",      CF_USE1|CF_USE2                 },  // multiply unsigned half-word data
    { "DIV0S",      CF_USE1                         },  // initial setting up for signed division
    { "DIV0U",      CF_USE1                         },  // initial setting up for unsigned division
    { "DIV1",       CF_USE1                         },  // main process of division
    { "DIV2",       CF_USE1                         },  // correction when remainder is 0
    { "DIV3",       0                               },  // correction when remainder is 0
    { "DIV4S",      0                               },  // correction answer for signed division
    { "LSL",        CF_USE1|CF_USE2|CF_CHG2|CF_SHFT },  // logical shift to the left direction
    { "LSL2",       CF_USE1|CF_USE2|CF_CHG2|CF_SHFT },  // logical shift to the left direction
    { "LSR",        CF_USE1|CF_USE2|CF_CHG2|CF_SHFT },  // logical shift to the right direction
    { "LSR2",       CF_USE1|CF_USE2|CF_CHG2|CF_SHFT },  // logical shift to the right direction
    { "ASR",        CF_USE1|CF_USE2|CF_CHG2|CF_SHFT },  // arithmetic shift to the right direction
    { "ASR2",       CF_USE1|CF_USE2|CF_CHG2|CF_SHFT },  // arithmetic shift to the right direction
    { "LDI:32",     CF_USE1|CF_USE2|CF_CHG2         },  // load immediate 32-bit data to destination register
    { "LDI:20",     CF_USE1|CF_USE2|CF_CHG2         },  // load immediate 20-bit data to destination register
    { "LDI:8",      CF_USE1|CF_USE2|CF_CHG2         },  // load immediate 8-bit data to destination register
    { "LD",         CF_USE1|CF_USE2|CF_CHG2         },  // load word data in memory to register / program status register
    { "LDUH",       CF_USE1|CF_USE2|CF_CHG2         },  // load half-word data in memory to register
    { "LDUB",       CF_USE1|CF_USE2|CF_CHG2         },  // load byte data in memory to register
    { "ST",         CF_USE1|CF_USE2|CF_CHG2         },  // store word data in register / program status register to memory
    { "STH",        CF_USE1|CF_USE2|CF_CHG2         },  // store half-word data in register to memory
    { "STB",        CF_USE1|CF_USE2|CF_CHG2         },  // store byte data in register to memory
    { "MOV",        CF_USE1|CF_USE2|CF_CHG2         },  // move word data in source register / program status register to destination register / program status register
    { "JMP",        CF_USE1|CF_STOP|CF_JUMP         },  // jump
    { "CALL",       CF_USE1|CF_CALL                 },  // call subroutine
    { "RET",        CF_STOP                         },  // return from subroutine
    { "INT",        CF_USE1                         },  // software interrupt
    { "INTE",       0                               },  // software interrupt for emulator
    { "RETI",       CF_STOP                         },  // return from interrupt
    { "BRA",        CF_USE1|CF_STOP                 },  // branch relative if condition satisfied
    { "BNO",        CF_USE1                         },  // branch relative if condition satisfied
    { "BEQ",        CF_USE1                         },  // branch relative if condition satisfied
    { "BNE",        CF_USE1                         },  // branch relative if condition satisfied
    { "BC",         CF_USE1                         },  // branch relative if condition satisfied
    { "BNC",        CF_USE1                         },  // branch relative if condition satisfied
    { "BN",         CF_USE1                         },  // branch relative if condition satisfied
    { "BP",         CF_USE1                         },  // branch relative if condition satisfied
    { "BV",         CF_USE1                         },  // branch relative if condition satisfied
    { "BNV",        CF_USE1                         },  // branch relative if condition satisfied
    { "BLT",        CF_USE1                         },  // branch relative if condition satisfied
    { "BGE",        CF_USE1                         },  // branch relative if condition satisfied
    { "BLE",        CF_USE1                         },  // branch relative if condition satisfied
    { "BGT",        CF_USE1                         },  // branch relative if condition satisfied
    { "BLS",        CF_USE1                         },  // branch relative if condition satisfied
    { "BHI",        CF_USE1                         },  // branch relative if condition satisfied
    { "DMOV",       CF_USE1|CF_USE2|CF_CHG2         },  // move word data from register / address to register / address
    { "DMOVH",      CF_USE1|CF_USE2|CF_CHG2         },  // move half-word data from register / address to register / address
    { "DMOVB",      CF_USE1|CF_USE2|CF_CHG2         },  // move byte data from register / address to register / address
    { "LDRES",      CF_USE1|CF_USE2|CF_CHG1         },  // load word data in memory to resource
    { "STRES",      CF_USE1|CF_USE2|CF_CHG1         },  // store word data in resource to memory
    { "COPOP",      CF_USE1|CF_USE2|CF_USE3|CF_USE4 },  // coprocessor operation
    { "COPLD",      CF_USE1|CF_USE2|CF_USE3|CF_USE4|CF_CHG4 },  // load 32-bit data from register to coprocessor register
    { "COPST",      CF_USE1|CF_USE2|CF_USE3|CF_USE4|CF_CHG4 },  // store 32-bit data from coprocessor register to register
    { "COPSV",      CF_USE1|CF_USE2|CF_USE3|CF_USE4|CF_CHG4 },  // save 32-bit data from coprocessor register to register
    { "NOP",        0                               },  // no operation
    { "ANDCCR",     CF_USE1                         },  // and condition code register and immediate data
    { "ORCCR",      CF_USE1                         },  // or condition code register and immediate data
    { "STILM",      CF_USE1                         },  // set immediate data to interrupt level mask register
    { "ADDSP",      CF_USE1                         },  // add stack pointer and immediate data
    { "EXTSB",      CF_USE1|CF_CHG1                 },  // sign extend from byte data to word data
    { "EXTUB",      CF_USE1|CF_CHG1                 },  // unsign extend from byte data to word data
    { "EXTSH",      CF_USE1|CF_CHG1                 },  // sign extend from byte data to word data
    { "EXTUH",      CF_USE1|CF_CHG1                 },  // unsigned extend from byte data to word data
    { "SRCH0",      CF_USE1|CF_CHG1                 },  // search first zero bit position distance from MSB
    { "SRCH1",      CF_USE1|CF_CHG1                 },  // search first one bit position distance from MSB
    { "SRCHC",      CF_USE1|CF_CHG1                 },  // search first bit value change position distance from MSB
    { "LDM0",       CF_USE1                         },  // load multiple registers
    { "LDM1",       CF_USE1                         },  // load multiple registers
    { "STM0",       CF_USE1                         },  // store multiple registers
    { "STM1",       CF_USE1                         },  // store multiple registers
    { "ENTER",      CF_USE1                         },  // enter function
    { "LEAVE",      CF_USE1                         },  // leave function
    { "XCHB",       CF_CHG1|CF_CHG2                 }   // exchange byte data
};

CASSERT(qnumber(Instructions) == fr_last);
