#ifndef __PARSE_H__
#define __PARSE_H__

#include <string>
#include <stack>
#include <iostream>
#include "LR1.h"
#include "token.h"

class parse{

  //lr1 provide prodRule & oracle
  LR1 lr1;
	
  std::stack<std::string> stateStack;

  token* reduceTok;
  token* curTok;
  std::vector<token*> paTree;
  int source; //0 bof, 1 stream, 2 eof
  //counter, token read //cur state, from 0
  int numTokValid;
  std::string curState;
  std::string curInput;
  std::string firstAct;
  std::string secondAct; 

public:
  parse(std::istream& grammar);
  std::string& getCurInput();
    
  void reduceTree(int rule);   	
  bool updateInput(std::istream& in);
  bool updateAction(std::string& input) throw(std::string);
  bool canReduce();
  bool reduce();
  void popXState(int x);
  void reduceShift();
  void shift();

  bool isEnd();
  void printErr();
  void printTree();
  void printRule(int n);
  ~parse();
};

#endif
