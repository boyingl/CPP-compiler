#include "parse.h"
#include "token.h"
#include "LR1.h"
#include <vector>
#include <stack>
#include <string>
#include <sstream>
#include <fstream>

using namespace std;

parse::parse(istream& grammar): 
lr1{grammar}, reduceTok{nullptr}, curTok{nullptr}, source{0} ,numTokValid{0}{
  //stack of states, sequence of tokens to be read
  stateStack.push("0");
}


string& parse::getCurInput(){
  return curInput;
}


bool parse::updateInput(istream& in){
  if(source == 2)  return false;
  curTok = new token();
  curTok->updateTok(in, source);
  curInput = curTok->type;
  return true;
}


//update the action base on input
bool parse::updateAction(string& input) throw(string){
  if(stateStack.size() == 0) throw string("ERROR: try to get cur state, but stateStack is empty" );
  curState = stateStack.top();

  if(! lr1.isInOracle(curState, input))  return false;

  istringstream fullAction {lr1.getAction(curState, input)};
  fullAction >> firstAct >> secondAct;
  return true;
}


bool parse::canReduce(){
  return "reduce" == firstAct;
}


//create a subtree based on reduce rule
//lhs is the root and rhs are children
void parse::reduceTree(int n){
  reduceTok = new token();
  reduceTok->hasKid = true;

  ostringstream theRule;
  theRule << lr1.prodRule[n][0];
  unsigned int rulSize = lr1.prodRule[n].size();
  for(unsigned int i = 1; i < rulSize; i ++){
    theRule <<" " << lr1.prodRule[n][i]; 
  }
  reduceTok->rule = theRule.str();

  //rhs token in the paTree becomes the new subtree's kids
  vector<token*>::iterator it = paTree.end() - (rulSize -1);
  for( ;it != paTree.end(); it++){
    reduceTok->kid.push_back(*it); 
  }
  
  //pop the rhs token from paTree
  for(unsigned int i = 0; i < (rulSize-1); i++) paTree.pop_back();
  reduceTok->type = lr1.prodRule[n][0];
}


bool parse::reduce(){
  int n;
  istringstream isint {secondAct};
  isint >> n;

  int rulSize = lr1.prodRule[n].size();
  popXState(rulSize - 1);
  reduceTree(n);
  if( ! updateAction(lr1.prodRule[n][0]))  return false;

  //shift lhs of the rule
  reduceShift();
  return true;
}


void parse::popXState(int x){
  for(int i = 0; i < x; i ++)  stateStack.pop();
}


//shift reduceTok on the paTree
void parse::reduceShift(){
  stateStack.push(secondAct);
  paTree.push_back(reduceTok);
  reduceTok = nullptr;
}


//shift curTok on the paTree
void parse::shift(){
  stateStack.push(secondAct);
  numTokValid ++;
  paTree.push_back(curTok);
  curTok = nullptr;
}


bool parse::isEnd(){
  return "EOF" == curInput;
}


void parse::printErr(){
  //no need to +1, since numTokValid include BOF
  cerr << "ERROR at " << numTokValid << endl;
}


//print paTree
void parse::printTree(){
  for(auto n : paTree) n->printTok();
}


//print production rule
void parse::printRule(int n){
  cout << lr1.prodRule[n][0];
  for(unsigned int i = 1; i < lr1.prodRule[n].size(); i++){
    cout << " " << lr1.prodRule[n][i];
  }
  cout << endl;
}


parse::~parse(){
  if(curTok)  delete curTok;
  if(reduceTok) delete reduceTok;
  if(!paTree.empty())  for(auto it : paTree)  delete it;
}
