//skip 2 lines
//read in line

//make a token with first string kind
//if is terminal, read in the second string
//if nonterminal, read in rest as kid(make token), then 
//each kid: 
//if terminal, getline, get second as lexeme
//if nonterm, getline, rule from second token, update each kid

#include "patree.h"
#include "token.h"
#include <iostream>
using namespace std;

void paTree::skipLine(istream& in, int num){
  string line;
  for(int i = 0; i < num; i ++) getline(in, line);
  return;
}

paTree::paTree(istream& in):source{in}{ }

void paTree::buildTree(){
	skipLine(source, 2);
	patree = new token("procedures");
	patree->fill(source);
}

void paTree::printTree(){
	patree->printTok();
}


paTree::~paTree(){
	if(patree) delete patree;
}

