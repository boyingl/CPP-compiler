#include "twopass.h"
#include <sstream>
#include <stdio.h>

using std::string;
using std::vector;
using std::endl;
using std::cerr;
using std::cin;
using std::cout;
using std::map;
using std::ostringstream;
using ASM::Token;
using ASM::IntToken;
typedef map<string,int> symbolTable;  //multiple times?

//store and remove label
void storeLabel (vector< vector<Token*> > & tokLines, symbolTable & st) throw (string){
  int PC = 0;
  vector< vector<Token*> >::iterator it;
  for (it = tokLines.begin(); it != tokLines.end(); ++it){ //for each line of tokens, 'it' is each line
    while(true){ 
      if(it->begin() == it->end()){ //empty line, erase, break while back to for loop next line
        tokLines.erase(it);
        --it;
        break;
      }
      
      Token first = **(it->begin());
      if("LABEL" == first.toString()){ //toStringget token catagory in cap-string
        string label = first.getLexeme(); //getLexeme gets the actual string
        label.pop_back();
        //if not found, store, erase
        if (st.find(label) == st.end()){
          st[label] = PC;
          delete *(it->begin());
          it->erase(it->begin()); 
          continue; //go back to while loop
         }
        else throw string("ERROR duplicate label");
      }
     
      if("DOTWORD" == first.toString()){
         PC += 4;
         break;
      }

      else throw string("ERROR during scanning and storing labels");
    }
  }
}

void labelToAddr(Token& tok, const symbolTable & st) throw(string){
  string labelOperand = tok.getLexeme();
  if(st.find(labelOperand) == st.end()) throw string("ERROR undefined label");
  int addr = st.at(labelOperand); 
  ostringstream os;
  if(os << addr) tok.replaceLabel(os.str());
  //cerr << "address is " << os.str() << endl; }
  else throw string("code ERROR truth table not r/w correctly");  
}

//only for .word
//replace second operand with its addr int if it is a label
void replaceLabel (vector< vector<Token*> > & tokLines,const symbolTable & st) throw(string){
  vector< vector<Token*> >::iterator it;
  for (it = tokLines.begin(); it != tokLines.end(); ++it){
    if(2 != it->size()) throw string("ERROR wrong number of tokens");
    Token* second = *(++it->begin());
    if("ID" == second->toString()){
    // cerr << "call labelToAddr\n"; 
     labelToAddr(*second, st);
     // string labelOperand = second->getLexeme();
     // if(st.find(labelOperand) != st.end()) throw "ERROR undefined label";
     // second ->changeToken(st.at(labelOperand));
    }
  }
}
//wordIOnly
//produce binary code
void wordIOnly (const vector< vector<Token*> > & tokLines) throw(string){
  vector< vector<Token*> >::const_iterator it;
  for (it = tokLines.begin(); it != tokLines.end(); ++it){
   // cout << "hello\n";
    if(2 != it->size()) throw string("ERROR wrong number of tokens");
    Token* first = *(it->begin());
    Token* second = *(++it->begin());
    if("DOTWORD" != first->toString()) throw string("ERROR wrong instruction");
    if("INT" != second->toString() && "HEXINT" != second->toString()) throw string("ERROR wrong type of operand");
    //scanner will check for out of bound int, hexint  //int only 4 bytes??? why okay??
    int i = second->toInt();
    putchar (i >> 24); 
    putchar (i >> 16);
    putchar (i >> 8);
    putchar (i);
  }
}

//print symbol table to stderr, one label-value pair per line
void printST(const symbolTable & st){
  symbolTable::const_iterator it;
  for (it = st.begin(); it != st.end(); ++it){
    cerr << it->first << " " << it->second << endl;  //make sure value is in dec!!
  }
}
