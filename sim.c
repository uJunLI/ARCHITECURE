#include <stdio.h>
#include "shell.h"


void process_instruction()
{
    /* execute one instruction here. You should use CURRENT_STATE and modify
     * values in NEXT_STATE. You can call mem_read_32() and mem_write_32() to
     * access memory. */
    uint32_t instruction = mem_read_32(CURRENT_STATE.PC);
    //printf("%x\n",instruction);
    //SYSCALL
    if (instruction == 12) {
        if (CURRENT_STATE.REGS[2] == 0x0A)
            RUN_BIT = 0;
        else {
            NEXT_STATE.PC = CURRENT_STATE.PC + 4;
        }
        return;
    }
    // op 6-bit operation code
    uint32_t op = instruction >> 26;
    uint32_t sa = (instruction >> 6) % 32;
    uint32_t rd = (instruction >> 11) % 32;
    uint32_t rt = (instruction >> 16) % 32;
    uint32_t rs = (instruction >> 21) % 32;
    uint16_t immediate = (uint16_t)instruction;
    uint32_t funct = instruction % 64;
    uint16_t offset = (uint16_t)instruction;

    uint32_t target;
    uint32_t vAddr;
    uint32_t data;
    uint16_t data2;
    uint8_t data1;
    

    // 下一条如果不是跳转指令则+4，否则之后再处理
    NEXT_STATE.PC = CURRENT_STATE.PC + 4;
    switch (op) {
        // addiu
    case 9:
        NEXT_STATE.REGS[rt] = (uint32_t)((int32_t)CURRENT_STATE.REGS[rs] + (int32_t)(int16_t)immediate);
        break;
        // addi 
    case 8:
        NEXT_STATE.REGS[rt] = (uint32_t)((int32_t)CURRENT_STATE.REGS[rs] + (int32_t)(int16_t)immediate);
        break;
        // ANDI
    case 12:
        NEXT_STATE.REGS[rt] = (uint32_t)(immediate & (uint16_t)(CURRENT_STATE.REGS[rs]));
        break;
        // BEQ
    case 4:
        // P25
        target = ((uint32_t)(int16_t)immediate) << 2;
        if (CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt])
            NEXT_STATE.PC = target + CURRENT_STATE.PC;
        break;
    //LUI
    case 15:
        NEXT_STATE.REGS[rt] = (uint32_t)immediate << 16;
        break;
    case 1:
        //BGEZ
        if (rt == 1) {
            target = ((uint32_t)(int16_t)immediate) << 2;
            if (CURRENT_STATE.REGS[rs] >> 31 == 0)
                NEXT_STATE.PC = target + CURRENT_STATE.PC;
        }
        //BGEZAL
        else if (rt == 17) {
            target = ((uint32_t)(int16_t)immediate) << 2;
            if (CURRENT_STATE.REGS[rs] >> 31 == 0)
                NEXT_STATE.PC = target + CURRENT_STATE.PC;
            NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
        }
        //BLTZ
        else if (rt == 0) {
            target = ((uint32_t)(int16_t)immediate) << 2;
            if (CURRENT_STATE.REGS[rs] >> 31 == 1)
                NEXT_STATE.PC = target + CURRENT_STATE.PC;
        }
        //BLTZAL
        else if (rt == 16) {
            target = ((uint32_t)(int16_t)immediate) << 2;
            if (CURRENT_STATE.REGS[rs] >> 31 == 1)
                NEXT_STATE.PC = target + CURRENT_STATE.PC;
            NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
        }
        break;
        //BGTZ
    case 7:
        if (CURRENT_STATE.REGS[rs] >> 31 == 0 && CURRENT_STATE.REGS[rs] != 0)
            NEXT_STATE.PC = CURRENT_STATE.PC + (uint32_t)((int16_t)immediate << 2);
        break;
        //BLEZ
    case 6:
        if (CURRENT_STATE.REGS[rs] >> 31 == 1 || CURRENT_STATE.REGS[rs] == 0x00000000)
            NEXT_STATE.PC = CURRENT_STATE.PC + (uint32_t)((int16_t)immediate << 2);
        break;
        //BNE
    case 5:
        if (CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt])
            NEXT_STATE.PC = CURRENT_STATE.PC + (uint32_t)((int16_t)immediate << 2);
        break;
        //J
    case 2:
        target = (instruction << 6) >> 4 | (CURRENT_STATE.PC >> 28) << 28;
        NEXT_STATE.PC = target;
        break;
        //JAL
    case 3:
        target = (instruction << 6) >> 4 | (CURRENT_STATE.PC >> 28) << 28;
        NEXT_STATE.REGS[31] = CURRENT_STATE.PC + 4;
        NEXT_STATE.PC = target;
        break;
    case 0:
        //JR
        if (funct == 8) {
            NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
        }
        //JALR
        else if (funct == 9) {
            NEXT_STATE.REGS[rd] = CURRENT_STATE.PC + 4;
            NEXT_STATE.PC = CURRENT_STATE.REGS[rs];
        }
        //ADD
        else if (funct == 32) {
            NEXT_STATE.REGS[rd] = (uint32_t)((int32_t)CURRENT_STATE.REGS[rs] + (int32_t)CURRENT_STATE.REGS[rt]);
        }
        //ADDU
        else if (funct == 33) {
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] + CURRENT_STATE.REGS[rt];
        }
        //AND
        else if (funct == 36) {
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt];
        }
        //DIV
        else if (funct == 26) {
            NEXT_STATE.LO = (uint32_t)((int32_t)CURRENT_STATE.REGS[rs] / (int32_t)CURRENT_STATE.REGS[rt]);
            NEXT_STATE.HI = (uint32_t)((int32_t)CURRENT_STATE.REGS[rs] % (int32_t)CURRENT_STATE.REGS[rt]);
        }
        //DIVU
        else if (funct == 27) {
            NEXT_STATE.LO = (uint32_t)CURRENT_STATE.REGS[rs] / (uint32_t)CURRENT_STATE.REGS[rt];
            NEXT_STATE.HI = (uint32_t)CURRENT_STATE.REGS[rs] % (uint32_t)CURRENT_STATE.REGS[rt];
        }
        //MFHI
        else if (funct == 16) {
            NEXT_STATE.REGS[rd] = CURRENT_STATE.HI;
        }
        //MFLO
        else if (funct == 18) {
            NEXT_STATE.REGS[rd] = CURRENT_STATE.LO;
        }
        //MTHI
        else if (funct == 17) {
            NEXT_STATE.HI = CURRENT_STATE.REGS[rs];
        }
        //MTLO
        else if (funct == 19) {
            NEXT_STATE.LO = CURRENT_STATE.REGS[rs];
        }
        //MULT
        else if (funct == 24) {
            int64_t t1 = (int64_t)(int32_t)CURRENT_STATE.REGS[rs] * (int64_t)(int32_t)CURRENT_STATE.REGS[rt];
            int64_t t2 = t1;
            NEXT_STATE.LO = (uint32_t)t1;
            NEXT_STATE.HI = (uint32_t)(t2 >> 32);
        }
        //MULTU 
        else if (funct == 25) {
            uint64_t t1 = (uint64_t)CURRENT_STATE.REGS[rs] * (uint64_t)CURRENT_STATE.REGS[rt];
            uint64_t t2 = t1;
            NEXT_STATE.LO = (uint32_t)t1;
            NEXT_STATE.HI = (uint32_t)(t2 >> 32);
        }
        //NOR
        else if (funct == 39) {
            NEXT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs] & ~CURRENT_STATE.REGS[rt]);
        }
        //OR
        else if (funct == 37) {
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs] | CURRENT_STATE.REGS[rt];
        }
        //SLL
        else if (funct == 0) {
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << sa;
        }
        //SLLV
        else if (funct == 4) {
            uint32_t temp = CURRENT_STATE.REGS[rs] << 28 >> 28;
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] << temp;
        }
        //SLT 有符号
        else if (funct == 42) {
            if ((int32_t)CURRENT_STATE.REGS[rs] < (int32_t)CURRENT_STATE.REGS[rt]) {
                NEXT_STATE.REGS[rd] = 1;
            }
            else {
                NEXT_STATE.REGS[rd] = 0;
            }
        }
        //SLTU 无符号
        else if (funct == 43) {
            if (CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt]) {
                NEXT_STATE.REGS[rd] = 1;
            }
            else {
                NEXT_STATE.REGS[rd] = 0;
            }
        }
        //SRA
        else if (funct == 3) {
            NEXT_STATE.REGS[rd] = (uint32_t)((int32_t)(CURRENT_STATE.REGS[rt]) >> sa);
        }
        //SRAV
        else if (funct == 7) {
            uint32_t temp = CURRENT_STATE.REGS[rs] << 27 >> 27;
            NEXT_STATE.REGS[rd] = (uint32_t)((int32_t)(CURRENT_STATE.REGS[rt]) >> temp);
        }
        //SRL
        else if (funct == 2) {
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> sa;
        }
        //SRLV
        else if (funct == 6) {
            uint32_t temp = CURRENT_STATE.REGS[rs] << 27 >> 27;
            NEXT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt] >> temp;
        }
        //SUB
        else if (funct == 34) {
            NEXT_STATE.REGS[rd] = (uint32_t)((int32_t)CURRENT_STATE.REGS[rs] - (int32_t)CURRENT_STATE.REGS[rt]);
        }
        //SUBU
        else if (funct == 35) {
            NEXT_STATE.REGS[rd] = (uint32_t)((int32_t)CURRENT_STATE.REGS[rs] - (int32_t)CURRENT_STATE.REGS[rt]);
        }
        //XOR
        else if (funct == 38) {
            NEXT_STATE.REGS[rd] = (~CURRENT_STATE.REGS[rs] & CURRENT_STATE.REGS[rt]) | (CURRENT_STATE.REGS[rs] & ~CURRENT_STATE.REGS[rt]);
        }
        break;
        //ORI
    case 13:
        NEXT_STATE.REGS[rt] = (CURRENT_STATE.REGS[rs] >> 16 << 16) | ((uint32_t)immediate | CURRENT_STATE.REGS[rs] << 16 >> 16);
        break;
        //SB
    case 40:
        vAddr = (uint32_t)((int32_t)(int16_t)offset + (int32_t)CURRENT_STATE.REGS[rs]);
        data = CURRENT_STATE.REGS[rt] << 24 >> 24;
        mem_write_32(vAddr, data);
        break;
        //SH 
    case 41:
        vAddr = (uint32_t)((int32_t)(int16_t)offset + (int32_t)CURRENT_STATE.REGS[rs]);
        data = CURRENT_STATE.REGS[rt] << 16 >> 16;
        mem_write_32(vAddr, data);
        break;
        //SW
    case 43:
        vAddr = (uint32_t)((int32_t)(int16_t)offset + (int32_t)CURRENT_STATE.REGS[rs]);
        data = CURRENT_STATE.REGS[rt];
        //print("SW data: %d\n",data);
        mem_write_32(vAddr, data);
        break;
        //SLTI
    case 10:
        if ((int32_t)CURRENT_STATE.REGS[rs] < (int32_t)(int16_t)immediate)
            NEXT_STATE.REGS[rt] = 1;
        else
            NEXT_STATE.REGS[rt] = 0;
        break;
        //SLTIU
    case 11:
        if (CURRENT_STATE.REGS[rs] < (uint32_t)(int16_t)immediate)
            NEXT_STATE.REGS[rt] = 1;
        else
            NEXT_STATE.REGS[rt] = 0;
        break;
        //LB 有符号
    case 32:
        vAddr = (uint32_t)((int32_t)(int16_t)offset + (int32_t)CURRENT_STATE.REGS[rs]);
        data = mem_read_32((uint32_t)vAddr);
        // 取一个字节的数字,并进行符号位扩展
        data1 = (uint8_t)data;
        NEXT_STATE.REGS[rt] = (uint32_t)(int8_t)data1;
        break;
        //LBU 无符号
    case 36:
        vAddr = (uint32_t)((int32_t)(int16_t)offset + (int32_t)CURRENT_STATE.REGS[rs]);
        data = mem_read_32(vAddr);
        // 取一个字节的数字,并进行符号位扩展
        data1 = (uint8_t)data;
        NEXT_STATE.REGS[rt] = (uint32_t)data1;
        break;
        //LH 有符号
    case 33:
        vAddr = (uint32_t)((int32_t)(int16_t)offset + (int32_t)CURRENT_STATE.REGS[rs]);
        data = mem_read_32((uint32_t)vAddr);
        // 取一个字节的数字,并进行符号位扩展
        data2 = (uint16_t)data;
        NEXT_STATE.REGS[rt] = (uint32_t)(int16_t)data2;
        break;
        //LHU 无符号
    case 37:
        vAddr = (uint32_t)((int32_t)(int16_t)offset + (int32_t)CURRENT_STATE.REGS[rs]);
        data = mem_read_32(vAddr);
        // 取一个字节的数字,并进行符号位扩展
        data2 = (uint16_t)data;
        NEXT_STATE.REGS[rt] = (uint32_t)data2;
        break;
        //LW 有符号
    case 35:
        vAddr = (uint32_t)((int32_t)(int16_t)offset + (int32_t)CURRENT_STATE.REGS[rs]);
        data = mem_read_32(vAddr);
        //print("LW data1: %d\n",data);
        NEXT_STATE.REGS[rt] = data;
        break;
        //XORI
    case 14:
        NEXT_STATE.REGS[rt] = (~CURRENT_STATE.REGS[rs] & (uint32_t)immediate) | (CURRENT_STATE.REGS[rs] & ~(uint32_t)immediate);
        break;
    default:
    }
}

