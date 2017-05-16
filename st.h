#ifndef __ST_H__
#define __ST_H__

#include <map>
#include <utility>
#include <string>

class token;
//create empty st
//generate st by calling buildST on patree (the * token)
//check for lack of dcl with checkSymbol() on patree

class symbolTable{
	//symbol - <type, offset>
	//the int = 0 if not used, = 1 if used before delete
	//after delete, then fill, then int is meaningful rep
	std::map<std::string, std::pair<std::string, int> > st;	
	const unsigned int numReg = 17;
	const int startReg = 12;

	bool isExist(token* tok);

	void addSymbol(std::string symbol, std::string type) throw (std::string);

	//build & check duplication & update ID token type
	void buildST(token* tok)throw (std::string);
	//check for use without declaration
	void checkSymbol(token* tok)throw(std::string);

	std::string getType(token* tok);

  void fillRep();
  void deleteUnUsed();


public:
  //buildST -> checksymbol 
	void setupST(token* ptok) throw(std::string);  

	bool isExist(std::string symbol);
	std::string getType(std::string symbol);

	int getRep(std::string symbol);	
	int getRep(token* tok);
  //if all stored in reg
	bool isReg();
	int getNumOfVar();

	void printTable();
};

#endif

