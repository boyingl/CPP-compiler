#include <map>
#include <utility>
#include <string>
#include "st.h"
#include "token.h"
#include "patree.h"

using namespace std;



//buildST -> checksymbol 
void symbolTable::setupST(token* ptok) throw(string){
  buildST(ptok);
  checkSymbol(ptok);
  deleteUnUsed();
  fillRep();
}


//store all declared var and check for duplication
//also update the ID token's type to int or int*
void symbolTable::buildST(token* tok) throw (string){
	if( tok->isTerm()) 	return;
	
	else if (tok->getKind() == "dcl"){
		
		//type token, type (kid int or int *)
		int typeKid = tok->getKid()[0]->getKid().size();
		string type;
		if(typeKid == 1) type = "int";
		else type = "int*";
		
		//ID token, symbol
		token* theID = tok->getKid()[1];
		theID->setType(type);
		string symbol = theID->getLexeme();
	
		addSymbol(symbol, type); //will do duplication check
	}
	else 	for(auto &n : tok->getKid()) 		buildST(n); 
  return;

}

//check use without dcl(varify used) & update existing symbol's type
void symbolTable::checkSymbol(token* tok)throw(string){
	if( tok->isTerm()) return;
	else if (tok->getKind() == "factor"){
		//factor token's kid 
		vector<token*>& fkid = tok->getKid();
		if(fkid[0]->getKind() == "ID"){
			string varName = fkid[0]->getLexeme();
			if(fkid.size() != 1) throw "ERROR procedure symbol " + varName + " used without declaration";
			else{
				if (isExist(fkid[0])) {
					fkid[0]->setType(getType(fkid[0]));
			    //set varName symbol as used
			    st[varName].second = 1;					
					return;
				}
			    else throw "ERROR in factor->ID, the ID " + varName + " is not declared";
			}
		} 
	}

	else if (tok->getKind() == "lvalue"){
	
		//lvalue token's kid 
		vector<token*>& lkid = tok->getKid();
		string varName = lkid[0]->getLexeme();
		
		int numKid = lkid.size();
		if(numKid == 1){ //lvalue -> ID 
			if(isExist(lkid[0])) {
				//get symbol type, set to token type
				lkid[0]->setType(getType(lkid[0]));
			  //set varName symbol as used
			  st[varName].second = 1;
				return;
			}
			else throw "ERROR in lvalue->ID, the ID " + varName + " is not declared";
		}
	}
	
	for(auto &n : tok->getKid()) checkSymbol(n);
	
    return;

}

void symbolTable::deleteUnUsed(){
	auto it = st.begin();
	while(it != st.end()){
  	if(it->second.second) it++;
  	else st.erase(it++);
  }
}

//reg 12 to 28  -> 17
void symbolTable::fillRep(){
	int curRep, increment;
	if(isReg()){
	  curRep = startReg; 
	  increment = 1;
	}
	else{
		curRep = increment = -4;
	}
	for(auto &n : st){
		n.second.second = curRep;
		curRep += increment;
	}
}

void symbolTable::addSymbol(string symbol, string type) throw (string){
	if(isExist(symbol)) throw string("ERROR duplicate symbol ") + symbol;
	st[symbol] = make_pair(type, 0);
}



int symbolTable::getRep(string symbol){
	if(!isExist(symbol))  cerr << "ERROR call getRep, but symbol does not exist " << endl;
	return st[symbol].second;	
}	

//only ID token
int symbolTable::getRep(token* ptok){
	if(!isExist(ptok))  cerr << "ERROR call getRep, but symbol does not exist " << endl;
	string symbol = ptok->getLexeme();
	return st[symbol].second;	
}

string symbolTable::getType(string symbol){
	if(!isExist(symbol))  cerr << "ERROR call getType, but symbol does not exist " << endl;
	return st[symbol].first;
}

string symbolTable::getType(token* ptok){
	if(!isExist(ptok))  cerr << "ERROR call getType, but symbol does not exist " << endl;
	string symbol = ptok->getLexeme();
	return st[symbol].first;
}

bool symbolTable::isExist(string symbol){
	return st.find(symbol) != st.end();
}

bool symbolTable::isExist(token* ptok){
	if(ptok->getKind() != "ID")	cerr << "ERROR token is not ID type, cannot be a symobl" << endl;
	return st.find(ptok->getLexeme()) != st.end();
}

void symbolTable::printTable(){
	cerr << "wain" << endl;
	for(auto n : st) cerr << n.first << " " 
		               << n.second.first  << " " << n.second.second << endl;
}


bool symbolTable::isReg(){
	return false; //st.size() <= numReg;
}

int symbolTable::getNumOfVar(){
  return st.size();
}


