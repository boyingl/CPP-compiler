#include "token.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

using namespace std;


token::token(std::string kind) : isFilled{ false }, kind{ kind }, hasKid{ false } { }


bool token::isTerm() {
  //first char in kind is upper case
	int n = kind[0];
	return ((n >= 65) && (n <= 90));
}


string token::getType() {
	if (!isTyped) deriveType();
	if (!isTyped) {
		cerr << "ERROR token " << kind <<" has no type but getType() is called" << endl;
		return " ";
	}
	else return type;
}


string token::getKind() {
	return kind;
}


string token::getLexeme() {
	if (!isFilled) cerr << "token not filled with lexeme when getLexeme() is called" << endl;
	return lexeme;
}



string token::getRule(){
	return rule;
}

int token::toInt() {
	istringstream is(lexeme);
	int num;
	is >> num;
	return num;
}


void token::fillNon(istream& in) {
	isFilled = true;
	hasKid = true;
	isTyped = false;
	string line;
	getline(in, line);
	istringstream theRule{ line };
	theRule >> rule;
	string word;

	while (theRule >> word) {
		rule += " " + word;
		kid.push_back(new token{ word });
	}

	for (auto &n : kid) n->fill(in);
}


vector<token*>& token::getKid() {
	return kid;
}

void token::fillTerm(istream& in) {
	isFilled = true;
	string line;
	getline(in, line);
	istringstream is{ line };
	is >> line >> lexeme;
	if (kind == "NUM") {
		type = "int";
		isTyped = true;
	}
	else if (kind == "NULL") {
		type = "int*";
		isTyped = true;
	}
	else isTyped = false;
}


void token::setType(string t) {
	isTyped = true;
	type = t;
}


void token::fill(istream& in) {
	if (isTerm()) fillTerm(in);
	else fillNon(in);
}

//do partial check as well, check for types not fitting type derivation rule
void token::deriveType() throw(string) {

	//set type if token has a type then derive the subtree
	if (isTyped || (!hasKid))  return;

  //give dcl & type type for easier type check
	if (kind == "dcl")  type = kid[0]->getType();

	else if (kind == "type")  type = (kid.size() == 1 ? "int" : "int*");

	else if (kind == "expr") {
		if (kid.size() == 1) type = kid[0]->getType();
		else {
			string first = kid[0]->getType();
			string second = kid[2]->getType();
			if (first == second && first == "int") type = "int";
			else if (kid[1]->getKind() == "PLUS") {
				if (first == second) throw string("ERROR: cannot have int* + int* for expr ");
				else type = "int*";
			}
			else {
				if (first != "int*") throw string("ERROR: cannot have int - int* for expr ");
				if(second == "int") type ="int*";
				else type = "int";
			}
		}
	}
	
	else if (kind == "term") {
		if (kid.size() == 1) type = kid[0]->getType();
		else{
			string first = kid[0]->getType();
			string second = kid[2]->getType();
			if(first == second && first =="int") type="int";
			else throw string("ERROR: cannot have "+first+" "+second+" for term ");
		}
	}	

	else if (kind == "factor") {
		if (kid.size() == 1) type = kid[0]->getType();
		else if(kid.size() == 5){
			string expr = kid[3]->getType();
			if(expr == "int") type = "int*";
			else throw string("ERROR: cannot use new on int ");
		}

		else{
			string kidType = kid[1]->getType();
			if(kid.size() == 3) type = kidType;
			else if((kid[0]->getKind() == "AMP") && (kidType=="int")) type = "int*";
			else if((kid[0]->getKind() == "STAR") && (kidType=="int*")) type = "int";
			else throw string("ERROR addrOf or dereference mismatch");			
		}
			//	cout << kind << " is of type " << type << endl;


	}
	else if (kind == "lvalue") {
		//cout << "   lvalue rule is " << rule << endl;
		
		if (kid.size() == 1) type = kid[0]->getType();
		else{
			string kidType = kid[1]->getType();		
			if(kid.size() == 3) type = kidType;
			else{
				if("int*" == kidType) type = "int";
				else throw string("ERROR: cannot dereference int");			
			}
		}

//cout << kind << " is of type " << type << endl;

	}
	isTyped = true;
	for (auto& n : kid)  n->deriveType();
		//	if(kind == "expr" || kind == "dcl"|| kind == "factor"|| kind == "term"|| kind == "lvalue" )cout << "exit derive " << kind << " type is " << type <<endl << endl;

		return;
}


void token::checkType() throw(string) {
	if(!hasKid) return;
	if (kind == "main") {
		if(kid[5]->getType() != "int") throw string("ERROR second dcl in main must be int");
		if(kid[11]->getType() != "int") throw string("ERROR wain has to return int");
	}

	else if (kind == "dcls") {
		//cout << "  check dcls type, rule is " << rule << endl;

		if(kid.size() == 5){
			if(kid[3]->getKind() == "NUM"){
				if(!kid[1]->matchType("int")) throw string("ERROR cannot assign num to int*");
			}
			else{
				if(!kid[1]->matchType("int*")) throw string("ERROR cannot assign null to int");
			}
		}

	}
	else if (kind == "statement") {
		if(kid.size() != 0){
			string kidKind = kid[0]->getKind();
			if(kidKind == "lvalue"){
				string first = kid[0]->getType();
				if(first != kid[2]->getType()) 	throw string("ERROR cannot assign a different type in stmt");
			}
			else if (kidKind == "PRINTLN"){
				if(kid[2]->getType() != "int") throw string("ERROR cannot println int*");
			}
			else if(kidKind == "DELETE"){
				if(kid[3]->getType() != "int*") throw string("ERROR cannot delete int");
			}
		}
	}
	else if (kind == "test") {
		string first = kid[0]->getType();
		string second = kid[2]->getType();
		if(first != second) throw string("ERROR cannot compare different type");
	}

	for(auto& n : kid) n->checkType();
		return;
}

//does full type check
void token::fullTypeCheck() throw(string){  
	deriveType();
  checkType();
}

bool token::matchType(string rightType) {
	return type == rightType;
}


//no kid print lexeme; has kid print rule
void token::printTok() {

	if (hasKid) {
		cout << kind <<  " type:" << type << endl;
		//cout  <<  rule  << endl;
		for (auto n : kid) n->printTok();
	}
	else  ;//cout << "kind: " << kind <<  " lexeme: " << lexeme  << endl;
}

token::~token() {
	if (hasKid)  for (auto it : kid) delete it;
}
