//
// Created by Naiqian on 2021/5/31.
//

#include "backend.h"
void process_tigger(std::string s, int type){
    if(print_flag5){
        dispatchTiggerExp(s, type);
        return;
    }
    if(print_flag4){
        fprintf(yyout, "%s\n", s.c_str());
    }
}
void process_riscv(std::string s){
    fprintf(yyout, "%s\n", s.c_str());
}

void dispatchTiggerOp(std::string op, std::string r1, std::string r2, std::string r3){
    std::string ans;
    if(op == "+"){
        ans = "add " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
    }
    else if(op == "-"){
        ans = "sub " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
    }
    else if(op == "*"){
        ans = "mul " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
    }
    else if(op == "/"){
        ans = "div " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
    }
    else if(op == "%"){
        ans = "rem " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
    }
    else if(op == "<"){
        ans = "slt " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
    }
    else if(op == ">"){
        ans = "sgt " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
    }
    else if(op == "<="){
        ans = "sgt " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
        ans = "seqz " + r1 + ", " + r1;
        process_riscv(ans);
    }
    else if(op == ">="){
        ans = "slt " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
        ans = "seqz " + r1 + ", " + r1;
        process_riscv(ans);
    }
    else if(op == "&&"){
        ans = "snez " + r1 + ", " + r2;
        process_riscv(ans);
        ans = "snez t0, "+r3;
        process_riscv(ans);
        ans = "and " + r1 + ", " + r1 + ", t0";
        process_riscv(ans);
    }
    else if(op == "||"){
        ans = "or " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
        ans = "snez " + r1 + ", " + r1;
        process_riscv(ans);
    }
    else if(op == "!="){
        ans = "xor " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
        ans = "snez " + r1 + ", " + r1;
        process_riscv(ans);
    }
    else if(op == "=="){
        ans = "xor " + r1 + ", " + r2 + ", " + r3;
        process_riscv(ans);
        ans = "seqz " + r1 + ", " + r1;
        process_riscv(ans);
    }
    else{
        fprintf(stderr, "dispatch Tigger Op type mismatch\n");
        exit(16);
    }
}

void dispatchTiggerSOp(std::string op, std::string r1, std::string r2){
    if(op == "-"){
        std::string ans = "neg "+r1+", "+r2;
        process_riscv(ans);
    }
    else if(op == "!"){
        std::string ans = "seqz "+r1+", "+r2;
        process_riscv(ans);
    }
    else{
        fprintf(stderr, "dispatch Tigger single op type mismatch\n");
        exit(17);
    }
}

void dispatchTiggerExp(std::string exp, int type){
    cerr << "[TIGGER Type: " << type << "] " << exp << endl;
    char c1, c2, c3;
    int n1, n2, n3;
    std::string s1, s2, s3;
    std::string ans;
    std::string op;
    char buf[500];
    memset(buf, 0, sizeof(buf));
    int stk = 0;
    switch(type){
        case 1:{
            sscanf(exp.c_str(), "v%d = %d", &n1, &n2);
            s1 = "v"+std::to_string(n1);
            process_riscv("  .global   "+s1);
            process_riscv("  .section  .sdata");
            process_riscv("  .align    2");
            process_riscv("  .type     " + s1 + ", @object");
            process_riscv("  .size     " + s1 + ", 4");
            process_riscv(s1+":");
            process_riscv("  .word     "+std::to_string(n2));
            break;
        }
        case 2:{
            sscanf(exp.c_str(), "v%d = malloc %d", &n1, &n2);
            s1 = "v"+std::to_string(n1);
            ans = "  .comm " + s1 + ", " + std::to_string(n2) + ", 4";
            process_riscv(ans);
            break;
        }
        case 3:{
            sscanf(exp.c_str(), "f_%s [%d] [%d]", buf, &n1, &n2);
            process_riscv(std::string("  .text\n")+
                          std::string("  .align  2"));
            ans = "  .global " + std::string(buf);
            process_riscv(ans);
            ans = "  .type   " + std::string(buf) + ", @function";
            process_riscv(ans);
            ans = std::string(buf) + ':';
            process_riscv(ans);
            stk = (n2 / 4 + 1) * 16;
            stksize = stk;
            if(abs(stk) <= 1024){
                ans = "  addi    sp, sp, -" + std::to_string(stk);
                process_riscv(ans);
            }
            else{
                ans = "  li t0, -" + std::to_string(stk);
                process_riscv(ans);
                ans = "  add sp, sp, t0";
                process_riscv(ans);
            }
            ans = "li t0, " + std::to_string(stk-4);
            process_riscv(ans);
            ans = "add t0, t0, sp";
            process_riscv(ans);
            ans = "  sw      ra, 0(t0)";
            process_riscv(ans);
            break;
        }
        case 4:{
            sscanf(exp.c_str(), "end f_%s", buf);
            ans = "  .size   " + std::string(buf) + ", .-" + std::string(buf);
            process_riscv(ans);
            break;
        }
        case 5:{
            sscanf(exp.c_str(), "%c%d = %c%d %s %c%d", &c1, &n1, &c2, &n2, buf, &c3, &n3);
            op = std::string(buf);
            memset(buf, 0, sizeof(buf)); buf[0] = c1;
            s1 = std::string(buf) + std::to_string(n1);
            memset(buf, 0, sizeof(buf)); buf[0] = c2;
            s2 = std::string(buf) + std::to_string(n2);
            memset(buf, 0, sizeof(buf)); buf[0] = c3;
            s3 = std::string(buf) + std::to_string(n3);
            dispatchTiggerOp(op, s1, s2, s3);
            break;
        }
        case 6:{
            sscanf(exp.c_str(), "%c%d = %c%d %s %d", &c1, &n1, &c2, &n2, buf, &n3);
            op = std::string(buf);
            memset(buf, 0, sizeof(buf)); buf[0] = c1;
            s1 = std::string(buf) + std::to_string(n1);
            memset(buf, 0, sizeof(buf)); buf[0] = c2;
            s2 = std::string(buf) + std::to_string(n2);
            ans = "li t0, " + std::to_string(n3);
            process_riscv(ans);
            dispatchTiggerOp(op, s1, s2, "t0");
            break;
        }
        case 7:{
            sscanf(exp.c_str(), "%c%d = %s %c%d", &c1, &n1, buf, &c2, &n2);
            op = std::string(buf);
            memset(buf, 0, sizeof(buf)); buf[0] = c1;
            s1 = std::string(buf) + std::to_string(n1);
            memset(buf, 0, sizeof(buf)); buf[0] = c2;
            s2 = std::string(buf) + std::to_string(n2);
            dispatchTiggerSOp(op, s1, s2);
            break;
        }
        case 8:{
            sscanf(exp.c_str(), "%c%d = %c%d", &c1, &n1, &c2, &n2);
            memset(buf, 0, sizeof(buf)); buf[0] = c1;
            s1 = std::string(buf) + std::to_string(n1);
            memset(buf, 0, sizeof(buf)); buf[0] = c2;
            s2 = std::string(buf) + std::to_string(n2);
            ans = "mv " + s1 + ", " + s2;
            process_riscv(ans);
            break;
        }
        case 9:{
            sscanf(exp.c_str(), "%c%d = %d", &c1, &n1, &n2);
            memset(buf, 0, sizeof(buf)); buf[0] = c1;
            s1 = std::string(buf) + std::to_string(n1);
            ans = "li " + s1 + ", " + std::to_string(n2);
            process_riscv(ans);
            break;
        }
        case 10:{
            sscanf(exp.c_str(), "%c%d[%d] = %c%d", &c1, &n1, &n2, &c3, &n3);
            memset(buf, 0, sizeof(buf)); buf[0] = c1;
            s1 = std::string(buf) + std::to_string(n1);
            memset(buf, 0, sizeof(buf)); buf[0] = c3;
            s3 = std::string(buf) + std::to_string(n3);
            ans = "sw " + s3 + ", " + std::to_string(n2) + "(" + s1 + ")";
            process_riscv(ans);
            break;
        }
        case 11:{
            sscanf(exp.c_str(), "%c%d = %c%d[%d]", &c1, &n1, &c2, &n2, &n3);
            memset(buf, 0, sizeof(buf)); buf[0] = c1;
            s1 = std::string(buf) + std::to_string(n1);
            memset(buf, 0, sizeof(buf)); buf[0] = c2;
            s2 = std::string(buf) + std::to_string(n2);
            ans = "lw " + s1 + ", " + std::to_string(n3) + "(" + s2 + ")";
            process_riscv(ans);
            break;
        }
        case 12:{
            sscanf(exp.c_str(), "if %c%d == x0 goto %s", &c1, &n1, buf);
            op = std::string(buf);
            memset(buf, 0, sizeof(buf)); buf[0] = c1;
            s1 = std::string(buf) + std::to_string(n1);
            ans = "beq " + s1 + ", x0, ." + op;
            process_riscv(ans);
            break;
        }
        case 13:{
            sscanf(exp.c_str(), "goto %s", buf);
            ans = "j ." + std::string(buf);
            process_riscv(ans);
            break;
        }
        case 14:{
            cerr << exp << endl;
            sscanf(exp.c_str(), "%s", buf);
            ans = "." + std::string(buf);
            process_riscv(ans);
            break;
        }
        case 15:{
            sscanf(exp.c_str(), "call f_%s", buf);
            ans = "call " + std::string(buf);
            process_riscv(ans);
            break;
        }
        case 16:{
//            sscanf(exp.c_str(), "return");
            ans = "li t0, " + std::to_string(stksize-4);
            process_riscv(ans);
            ans = "add t0, t0, sp";
            process_riscv(ans);
            ans = "lw ra, 0(t0)";
            process_riscv(ans);
            if(abs(stksize) <= 1024){
                ans = "addi sp, sp, " + std::to_string(stksize);
                process_riscv(ans);
            }
            else{
                ans = "li t0, " + std::to_string(stksize);
                process_riscv(ans);
                ans = "add sp, sp, t0";
                process_riscv(ans);
            }
            ans = "ret";
            process_riscv(ans);
            break;
        }
        case 17:{
            sscanf(exp.c_str(), "store %c%d %d", &c1, &n1, &n2);
            memset(buf, 0, sizeof(buf)); buf[0] = c1;
            s1 = std::string(buf) + std::to_string(n1);
            n2 = n2*4;
            if(abs(n2) <= 1024){
                ans = "sw " + s1 + ", " + std::to_string(n2) + "(sp)";
                process_riscv(ans);
            }
            else{
                ans = "li t0, " + std::to_string(n2);
                process_riscv(ans);
                ans = "add t0, t0, sp";
                process_riscv(ans);
                ans = "sw " + s1 + ", " + "0(t0)";
                process_riscv(ans);
            }
            break;
        }
        case 18:{
            sscanf(exp.c_str(), "load %d %c%d", &n1, &c2, &n2);
            memset(buf, 0, sizeof(buf)); buf[0] = c2;
            s2 = std::string(buf) + std::to_string(n2);
            n1 = n1*4;
            if(abs(n1) <= 1024){
                ans = "lw " + s2 + ", " + std::to_string(n1) + "(sp)";
                process_riscv(ans);
            }
            else{
                ans = "li t0, " + std::to_string(n1);
                process_riscv(ans);
                ans = "add t0, t0, sp";
                process_riscv(ans);
                ans = "lw " + s2 + ", " + "0(t0)";
                process_riscv(ans);
            }
            break;
        }
        case 19:{
            sscanf(exp.c_str(), "load %s %c%d", buf, &c1, &n1);
            op = std::string(buf);
            memset(buf, 0, sizeof(buf)); buf[0] = c1;
            s1 = std::string(buf) + std::to_string(n1);
            ans = "lui " + s1 + ", %hi(" + op + ")";
            process_riscv(ans);
            ans = "lw " + s1 + ", %lo(" + op + ")("+ s1 + ")";
            process_riscv(ans);
            break;
        }
        case 20:{
            sscanf(exp.c_str(), "loadaddr %d %c%d", &n1, &c2, &n2);
            memset(buf, 0, sizeof(buf)); buf[0] = c2;
            s2 = std::string(buf) + std::to_string(n2);
            n1 = n1*4;
            if(abs(n1) <= 1024){
                ans = "addi " + s2 + ", sp, " + std::to_string(n1);
                process_riscv(ans);
            }
            else{
                ans = "li t0, " + std::to_string(n1);
                process_riscv(ans);
                ans = "add "+s2+", sp, t0";
                process_riscv(ans);
            }
            break;
        }
        case 21:{
            sscanf(exp.c_str(), "loadaddr %s %c%d", buf, &c1, &n1);
            op = std::string(buf);
            memset(buf, 0, sizeof(buf)); buf[0] = c1;
            s1 = std::string(buf) + std::to_string(n1);
            ans = "la " + s1 + ", " + op;
            process_riscv(ans);
            break;
        }
        default:{
            fprintf(stderr, "Tigger Expression type mismatch\n");
            exit(11);
        }
    }
}
int stksize = 0;

