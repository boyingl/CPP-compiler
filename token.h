#ifndef __TOKEN_H__
#define __TOKEN_H__
#include <vector>
#include <string>



class token{
	//type of NULL & NUM filled at parse Tree build
	//type of ID created at symbol table build or at check symbol
	//type of expr, term, factor, lvalue at derive
	//typecheck: at checktype
  bool isFilled;
  std::string type;
  bool isTyped;
  std::string kind;
  std::string lexeme;
  std::vector <token*> kid;
  bool hasKid; //non-term has kid
  std::string rule;

public:
  
  token(std::string kind);
  bool isTerm();
  std::string getKind();
  std::string getLexeme();
  std::string getType();
  std::vector<token*>& getKid();
  std::string getRule();
  void setType(std::string t);
  int toInt();
  void deriveType() throw(std::string);

  void fullTypeCheck() throw(std::string);


  void fillNon(std::istream& in); //epsilon derivation, empty children
  void fillTerm(std::istream& in);
  void fill(std::istream& in);

  
  bool matchType(std::string rightType);
  void checkType() throw(std::string);

  //no kid print lexeme; has kid print rule
  void printTok();
  ~token();
};

#endif
