#ifndef __PATREE_H__
#define __PATREE_H__

#include <iostream>
#include <vector>

class token;

//create empty parse tree with source stream
//build parse tree by calling buildTree
class paTree{

	std::istream& source;
  void skipLine(std::istream& in, int num);

public:
	token* patree;
	paTree(std::istream& in);
	void buildTree();
	void printTree();
	~paTree();

};

#endif