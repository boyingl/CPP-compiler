#ifndef __TWOPASS_H__
#define __TWOPASS_H__
#include "kind.h"
#include "lexer.h"
#include <vector>
#include <string>
#include <iostream>
#include <map>

typedef std::map<std::string,int> symbolTable;
//storeLabel
//remove label at the same time
void storeLabel (std::vector< std::vector<ASM::Token*> > & tokenLines, symbolTable & st) throw(std::string);

void labelToAddr(ASM::Token& tok,const symbolTable &st) throw(std::string); 
//only for .word 
//replace the second operand if it is an label
void replaceLabel (std::vector< std::vector<ASM::Token*> > & tokenLines, const symbolTable & st) throw(std::string);

//wordIOnly
void wordIOnly (const std::vector< std::vector<ASM::Token*> > & tokenLines ) throw(std::string);

//print symbol table to stderr,one label-value pair each line
void printST(const symbolTable & st);

#endif
