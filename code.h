#ifndef __CODE_H__
#define __CODE_H__

#include <iostream>

class token;
class paTree;
class symbolTable;
//st: 
//call is in Reg, true then all rep is reg, false then offset
//

//convention:
//para & var stored in reg or as offset, para pushed at prologue, var pushed at dcls
//$4 = 4, $11 = 1, at prologue
//$5 tmp reg
//7 init, 8 new, 9 delete, 10 print, prologue


class code{
private:
  symbolTable& st;
  paTree& pa; //pa.patree is the root token
  std::ostream& codeStream;
  bool allReg;
  int numOfVar;
  int labelCounter;

  std::string prologue();
  std::string epilogue();
  std::string getCode(token* ptok);

  //use tok to generate comment (wlp4 code segment)
  std::string procedures(token* ptok);
  std::string mainCode(token* ptok);
  std::string nothing(token* ptok); //nothing or term or dcl, dcl is done in dcls & main
  std::string dcls(token* ptok);
  // std::string dcl(token* ptok);
  std::string statements(token* ptok);
  std::string statement(token* ptok);
  std::string test(token* ptok);
  std::string expr(token* ptok);
  std::string term(token* ptok);
  std::string factor(token* ptok);
  std::string lvalue(token* ptok); 

  std::string getLabel(); //print a able output: "label: "
  std::string move(int targetReg, int val, bool isReg); //move val to reg

  // //output string version of int
  // std::string intTOstring(int num);

  //code for store val at the offset wrt to $29 to reg
  std::string storeAtOffset(int reg, int offset);
  std::string storeAtRep(int value, int representation, bool isReg);

public:
  code(symbolTable& st, paTree& pa, std::ostream& codeStream);
  void fullCode();

};
#endif

