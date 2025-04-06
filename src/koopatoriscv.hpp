#pragma once
#include "koopa.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace std;

class Regcombo {
  public:
    std::string reg1;
    std::string reg2;
    std::string reg3;
    int reuse;
    Regcombo(std::string r1, std::string r2, std::string r3, int re = 0)
        : reg1(r1), reg2(r2), reg3(r3), reuse(re) {}

    Regcombo() : reg1(""), reg2(""), reg3(""), reuse(0) {}
    void DumpBinary(const koopa_raw_binary_t& binary){
      switch(binary.op){
        case KOOPA_RBO_ADD:
          std::cout<<"\tadd "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          break;
        case KOOPA_RBO_SUB:
          std::cout<<"\tsub "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          break;
        case KOOPA_RBO_MUL:
          std::cout<<"\tmul "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          break;
        case KOOPA_RBO_DIV:
          std::cout<<"\tdiv "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          break;
        case KOOPA_RBO_MOD:
          std::cout<<"\trem "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          break;
        case KOOPA_RBO_EQ:
          std::cout<<"\txor "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          std::cout<<"\tseqz "<<reg3<<", "<<reg3<<std::endl;
          break;
        case KOOPA_RBO_NOT_EQ:
          std::cout<<"\txor "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          std::cout<<"\tsnez "<<reg3<<", "<<reg3<<std::endl;
          break;
        case KOOPA_RBO_LT:
          std::cout<<"\tslt "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          break;
        case KOOPA_RBO_LE:
          std::cout<<"\tsgt "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          std::cout<<"\tseqz "<<reg3<<", "<<reg3<<std::endl;
          break;
        case KOOPA_RBO_GT:
          std::cout<<"\tsgt "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          break;
        case KOOPA_RBO_GE:
          std::cout<<"\tslt "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          std::cout<<"\tseqz "<<reg3<<", "<<reg3<<std::endl;
          break;
        case KOOPA_RBO_AND:
          std::cout<<"\tand "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          break;
        case KOOPA_RBO_OR:
          std::cout<<"\tor "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          break;
        case KOOPA_RBO_XOR:
          std::cout<<"\txor "<<reg3<<", "<<reg1<<", "<<reg2<<std::endl;
          break;
        default:
          std::cout<<"Unknown operator"<<std::endl;
          break;
      }
    }
};


class Regmanager {
  public:
    int regnum = 0;
    unordered_map<koopa_raw_value_t, std::string> regmap;

    std::string currreg(){
      std::string ret;
      if(regnum < 7)
        {ret = "t" + std::to_string(regnum);}
      else
        {ret = "a" + std::to_string(regnum-7);}
      regnum++;
      return ret;
    }

    void DumpBinary(const koopa_raw_binary_t& binary, const koopa_raw_value_t& value){
      Regcombo ret;
      koopa_raw_value_t lhs = binary.lhs;
      koopa_raw_value_t rhs = binary.rhs;
      if(lhs -> kind.tag == KOOPA_RVT_INTEGER){
        if(lhs -> kind.data.integer.value == 0){
          regmap[lhs] = "x0";
        }
        else{
          regmap[lhs] = currreg();
          std::cout<<"li "<<regmap[lhs]<<", "<<lhs -> kind.data.integer.value<<std::endl;
          ret.reuse = 1;
        }
      }
      if(rhs -> kind.tag == KOOPA_RVT_INTEGER){
        if(rhs -> kind.data.integer.value == 0){
          regmap[rhs] = "x0";
        }
        else{
          regmap[rhs] = currreg();
          std::cout<<"li "<<regmap[rhs]<<", "<<rhs -> kind.data.integer.value<<std::endl;
          ret.reuse = 2;
        }
      }
      ret.reg1 = regmap[lhs];
      ret.reg2 = regmap[rhs];
      switch(ret.reuse){
        case 0:
          ret.reg3 = currreg();
          break;
        case 1:
          ret.reg3 = ret.reg1;
          break;
        case 2:
          ret.reg3 = ret.reg2;
          break;
      }
      regmap[value] = ret.reg3;
      ret.DumpBinary(binary);
    }



};




void koopa_dump(const char* str);
void Visit(const koopa_raw_program_t &program);
void Visit(const koopa_raw_slice_t &slice);
void Visit(const koopa_raw_function_t &func);
void Visit(const koopa_raw_basic_block_t &bb);
void Visit(const koopa_raw_value_t &value);
void Visit(const koopa_raw_return_t& ret);