#pragma once

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <stack>

#define IMM 0
#define REG 1

using namespace std;

extern string mode;

static int currnum = 0;

class Result{
    public:
     int type = IMM;
     int value;
     friend ostream& operator<<(ostream& os, const Result& result) {
         if(result.type == IMM) {
             os << result.value;
         } else {
             os << "%" << result.value;
         }
         return os;
     }
        Result(int type, int value) : type(type), value(value) {}

};



// 所有 AST 的基类
class BaseAST {
    public:
    virtual ~BaseAST() = default;
   
    virtual Result Dump() const = 0;
};

class CompUnitAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> func_def;
   
     Result Dump() const override {
        return func_def->Dump();
     }
};
   
class FuncDefAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> func_type;
     std::string ident;
     std::unique_ptr<BaseAST> block;
   
     Result Dump() const override {
        std::cout << "fun @" << ident << "(): ";
        func_type->Dump();
        std::cout << " {";
        block->Dump();
        std::cout << "}" << endl;
        return Result(IMM, 0);
     }
};

class FuncTypeAST : public BaseAST {
    public:
     std::string type;
   
     Result Dump() const override {
        std::cout << "i32";
        return Result(IMM, 0);
     }
};

class BlockAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> stmt;
   
     Result Dump() const override {
        std::cout <<"%entry:" << endl;
        stmt->Dump();
        return Result(IMM, 0);
     }
};

class StmtAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> lval;
     std::unique_ptr<BaseAST> exp;
   
     Result Dump() const override {
        Result res = exp->Dump();
       std::cout << "\tret " << res <<endl;
         return Result(IMM, 0);
     }
};

class ExpAST : public BaseAST {
    public:
    std::unique_ptr<BaseAST> addexp;
   
     Result Dump() const override {
       return addexp->Dump();
     }
};

class PrimaryExpAST : public BaseAST {
    public:
     bool isnumber;
     std::unique_ptr<BaseAST> exp;
     std::unique_ptr<BaseAST> number;
   
     Result Dump() const override {
       if(isnumber) {
         return number->Dump();
       } else {
         return exp->Dump();
       }
     }
};

class NumberAST : public BaseAST {
    public:
     int value;
     explicit NumberAST(int v) : value(v) {}
     Result Dump() const override {
       Result res = Result(IMM, value);
        return res;
     }
};

class UnaryExpAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> unaryexp;
     std::unique_ptr<BaseAST> primaryexp;
     char op = '+';
     bool isprimary;
   
     Result Dump() const override {
        if(isprimary) {
          return primaryexp->Dump();
        } else {
            Result res = Result(REG, currnum);
            currnum++;
            Result temp_res = unaryexp->Dump();
            

            switch(op) {
                case '+':
                    std::cout << "\t" << res << " = add 0, "<<temp_res<<endl;
                    break;
                case '-':
                    std::cout << "\t" << res << " = sub 0, "<<temp_res<<endl;
                    break;
                case '!':
                    std::cout << "\t" << res << " = eq 0, "<<temp_res<<endl;
                    break;
                default:
                    break;
            }
            return res;
        }
     }
};

class AddExpAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> mulexp;
     std::unique_ptr<BaseAST> addexp;
     char op = '+';
     bool only = false;
   
     Result Dump() const override {
        if(!only) {
            Result res = Result(REG, currnum);
            currnum++;
            Result left_res = addexp->Dump();
            Result right_res = mulexp->Dump();

            switch(op) {
                case '+':
                    std::cout << "\t" << res << " = add "<<left_res<<", "<<right_res<<endl;
                    break;
                case '-':
                    std::cout << "\t" << res << " = sub "<<left_res<<", "<<right_res<<endl;
                    break;
                default:
                    break;
            }
            return res;
        } else {
            return mulexp->Dump();
        }
     }
};

class MulExpAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> unaryexp;
     std::unique_ptr<BaseAST> mulexp;
     char op = '*';
     bool only = false;
   
     Result Dump() const override {
        if(!only) {
            Result res = Result(REG, currnum);
            currnum++;
            Result left_res = mulexp->Dump();
            Result right_res = unaryexp->Dump();

            switch(op) {
                case '*':
                    std::cout << "\t" << res << " = mul "<<left_res<<", "<<right_res<<endl;
                    break;
                case '/':
                    std::cout << "\t" << res << " = div "<<left_res<<", "<<right_res<<endl;
                    break;
                case '%':
                    std::cout << "\t" << res << " = mod "<<left_res<<", "<<right_res<<endl;
                    break;
                default:
                    break;
            }
            return res;
        } else {
            return unaryexp->Dump();
        }
     }
};

class RelExpAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> addexp;
     std::unique_ptr<BaseAST> relexp;
     std::string op;
     bool only = false;
   
     Result Dump() const override {
        if(!only) {
            Result res = Result(REG, currnum);
            currnum++;
            Result left_res = relexp->Dump();
            Result right_res = addexp->Dump();

            if (op == "<") {
                std::cout << "\t" << res << " = lt " << left_res << ", " << right_res << std::endl;
            } else if (op == ">") {
                std::cout << "\t" << res << " = gt " << left_res << ", " << right_res<< std::endl;
            } else if (op == "<=") {
                std::cout << "\t" << res << " = le " << left_res << ", " <<right_res<< std::endl;
            } else if (op == ">=") {
                std::cout << "\t" << res << " = ge " << left_res << ", " << right_res<< std::endl;
            }

            return res;
        } else {
            return addexp->Dump();
        }
     }
};

class EqExpAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> relexp;
     std::unique_ptr<BaseAST> eqexp;
     std::string op;
     bool only = false;
   
     Result Dump() const override {
        if(!only) {
            Result res = Result(REG, currnum);
            currnum++;
            Result left_res = eqexp->Dump();
            Result right_res = relexp->Dump();

             if (op == "==") {
            std::cout << "\t" << res << " = eq " << left_res<< ", " << right_res << std::endl;
        } else if (op == "!=") {
            std::cout << "\t" << res << " = ne " << left_res<< ", " << right_res << std::endl;
        }

            return res;
        } else {
            return relexp->Dump();
        }
     }
};

class LAndExpAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> eqexp;
     std::unique_ptr<BaseAST> landexp;
     bool only = false;
   
     Result Dump() const override {
        if(!only) {
            Result res_1 = Result(REG, currnum);
            currnum++;
            Result res_2 = Result(REG, currnum);
            currnum++;
            Result res = Result(REG, currnum);
            currnum++;
            Result left_res = landexp->Dump();
            Result right_res = eqexp->Dump();

            std::cout << "\t" << res_1 << " = ne 0, "<<left_res<<endl;
            std::cout << "\t" << res_2 << " = ne 0, "<<right_res<<endl;
            std::cout << "\t" << res << " = and "<<res_1<<", "<<res_2<<endl;
            return res;
        } else {
            return eqexp->Dump();
        }
     }
};  

class LOrExpAST : public BaseAST {
    public:
     std::unique_ptr<BaseAST> landexp;
     std::unique_ptr<BaseAST> lorexp;
     bool only = false;
   
     Result Dump() const override {
        if(!only) {
            Result res_1 = Result(REG, currnum);
            currnum++;
            Result res_2 = Result(REG, currnum);
            currnum++;
            Result res = Result(REG, currnum);
            currnum++;
            Result left_res = lorexp->Dump();
            Result right_res = landexp->Dump();
            std::cout << "\t" << res_1 << " = ne 0, "<<left_res<<endl;
            std::cout << "\t" << res_2 << " = ne 0, "<<right_res<<endl;
            std::cout << "\t" << res << " = or "<<res_1<<", "<<res_2<<endl;
            return res;
        } else {
            return landexp->Dump();
        }
     }
};

