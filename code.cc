#include "code.h"
#include "token.h"
#include "patree.h"
#include "st.h"
#include <iostream>

using namespace std;

//st: 
//call is in Reg, true then all rep is reg, false then offset
//

//convention:
//para & var stored in reg or as offset, para pushed at prologue, var pushed at dcls
//$4 = 4, $11 = 1, at prologue
//$5 tmp reg lw from stack
//$6 tmp for test & stmt
//7 init, 8 new, 9 delete, 10 print, prologue


//prologue:
//push parameter ($1, $2) -> at main
//move sp to fp, offset of var is (-4, -8, -12 ...), 
  //do not push here(push at dcls), move sp to point to last var (sp - 4*var#)
//!!remeber fp offset & var, $31 mapping
string code::prologue(){
  //cout << "call prologue\n";
  //init, new, delete, print, 4, 1
  //if allReg, nothing
  //if not, offset, get #ofvar, move $30 to reserve frame (-4*var#)
  //push $31 ($31 at moved stack +4)
  //--> move stack together (-4 all reg, -4*var -4 offset)
  string theCode = ";prologue: \n";
  theCode += (".import print\n");
  theCode += (".import new\n");
  theCode += (".import delete\n");
  theCode += (".import init\n");
  theCode += ("lis $7\n .word init\n");
  theCode += ("lis $8\n .word new\n");
  theCode += ("lis $9\n .word delete\n");
  theCode += ("lis $10\n .word print\n");
  theCode += ("lis $4\n .word 4\n");
  theCode += ("lis $11\n .word 1\n");
  

  if(allReg) {
    theCode += "sw $31, -4($30)\n";
    theCode += "sub $30, $30, $4\n";
  }
  else{  
    //set frame pointer
    theCode += "add $29, $30, $0\n" ; 
    string moveStack = to_string (4 * numOfVar + 4);
    theCode += ("lis $5\n .word " + moveStack + "\n");
    theCode += "sub $30, $30, $5\n"; //move stack
    theCode += "sw $31, 0($30)\n";    
  }

  return theCode;
}


string code::epilogue(){
   //cout << "call epilogue\n";
  //load $31, restore stack(with var or not)
  string theCode = ";epilogue: \n";
  theCode += "lw $31, 0($30)\n";
  if(allReg) theCode += "sub $30, $30, $4\n";
  else {
    //put 4*var# + 4 into $5
    theCode += move(5, 4*numOfVar + 4, false);
    theCode += "sub $30, $30, $5\n";
  }
  theCode += "jr $31\n";
  return theCode;
}

string code::getCode(token* ptok){
  string tokKind = ptok->getKind();
  if("procedures" == tokKind) return procedures(ptok);
  if("main" == tokKind) return mainCode(ptok);
  if("dcls" == tokKind) return dcls(ptok);
  if("statements" == tokKind) return statements(ptok);
  if("statement" == tokKind) return statement(ptok);
  if("test" == tokKind) return test(ptok);
  if("expr" == tokKind) return expr(ptok);
  if("term" == tokKind) return term(ptok);
  if("factor" == tokKind) return factor(ptok);
  if("lvalue" == tokKind) return lvalue(ptok);
  else return nothing(ptok); //dcl, term, type
}

//use tok to generate comment (wlp4 code segment)
//mips comment is ;
// \n after comment
string code::procedures(token* ptok){  
  //cout << "call procedures\n";
  string theCode = "; code for procedure is :\n";
  //add mainCode, call it using main tok
  theCode += mainCode(ptok->getKid()[0]);
  return theCode;
}

//move $1, $2, call init
string code::mainCode(token* ptok){

  //cout << "call mainCode\n";
  string theCode = ";code for main\n";

  token* pDcl1 = ptok->getKid()[3];
  token* pDcl2 = ptok->getKid()[5];

  //handle para1 & para2
  string para1 = pDcl1->getKid()[1]->getLexeme();
  string para2 = pDcl2->getKid()[1]->getLexeme();
  //cout << "two parameters: " << para1 << " and " << para2 << endl;
  //if para1 used, handle para1
  if(st.isExist(para1)){
    //cout << "para 1 exists\n";
    int rep1 = st.getRep(para1);  
    if(allReg){
      theCode += ";representation is reg, move parameters\n";
      //rep1, rep2 is reg, move $1 to $rep1, move $2 to $rep2
      theCode += move(rep1, 1, true);
    }
    else{
      theCode += ";representation is offset, push para on frame\n";
      //sw $1, offset($29)
      theCode += storeAtOffset(1, rep1);
    }
  }

  if(st.isExist(para2)){
    //cout << "para 2 exists\n";
    int rep2 = st.getRep(para2);  
    if(allReg){
      theCode += ";representation is reg, move parameters\n";
      //rep1, rep2 is reg, move $1 to $rep1, move $2 to $rep2
      theCode += move(rep2, 2, true);
    }
    else{
      theCode += ";representation is offset, push para on frame\n";
      //sw $2, offset($29)
      theCode += storeAtOffset(2, rep2);
    }
  }
  

  //call init
   string type1 = pDcl1->getKid()[1]->getType();
  // //para1 int type, change $2 to 0, call init
   //cout << "before call init, first para is type: " << type1 << endl;
   if(type1 == "int")  theCode += "add $2, $0, $0\n";
  
  // //para1 int* type, nothing
  
  // //call init
   theCode += "jalr $7\n";
  
  //+ [dcls]8 [stmts]9 [expr]11
  theCode += dcls(ptok->getKid()[8]); 
  theCode += statements(ptok->getKid()[9]);
  theCode += expr(ptok->getKid()[11]);
  
  //cout << "exist main code \n";
  return theCode;
}


string code::nothing(token* ptok){ //nothing or term
  return "";
}


//[dcls] + push values of var on frame
string code::dcls(token* ptok){
  string theCode = "; code for dcls\n"; 

  //cout << "call dcls\n" <<  ptok->getRule() << ptok->getKid().size() << endl;
  if(ptok->getKid().size() == 0) return "";

  string rule = ptok->getRule();
  theCode += ("   ;;" + rule +"\n");
  
  theCode += dcls(ptok->getKid()[0]);
  token* pDcl = ptok->getKid()[1];
  //get representation of the var
  string para = pDcl->getKid()[1]->getLexeme();
  if(st.isExist(para)){
    int rep = st.getRep(para);
    int value;

    if("dcls dcls dcl BECOMES NUM SEMI" == rule){
      value = ptok->getKid()[3]->toInt();
      theCode += ("  ;the value of the var is " + to_string(value) + "\n");
    }
    else value = 0x01;

    //put value at rep of the var  
    theCode += storeAtRep(value, rep, false);
  }
  //cout << "exit dcls\n";
  return theCode;
}


// std::string dcl(token* ptok);


//[stmts] + other
string code::statements(token* ptok){

  string theCode = "; code for stmts\n"; 
  string rule = ptok->getRule();
  theCode += ("   ;;" + rule +"\n");
  //cout << "call statements\n" << rule <<endl;
  
  if("statements statements statement" == rule){
    theCode += statements(ptok->getKid()[0]);
    theCode += statement(ptok->getKid()[1]);
  }
  else ; //epsilon rule
  //cout << "exit statements\n";
  return theCode;
}
 

string code::statement(token* ptok){

  string theCode = "\n\n; code for stmt\n"; 
  string rule = ptok->getRule();
  theCode += ("   ;;" + rule +"\n");
  //cout << "call statement, rule: " << rule << endl;
  if("statement lvalue BECOMES expr SEMI" == rule){
    //store expr value at $3
    theCode += expr(ptok->getKid()[2]);

    //move $3 to tmp $6
    theCode += move(6, 3, true);
     //lvalue dont use tmp, wont tempt with tmp
    //get addr(rep in st) of lvalue at $3
    theCode += lvalue(ptok->getKid()[0]);
    theCode += "sw $6, 0($3)\n";
  }

  //not safe, assume branch i okay
  else if("statement IF LPAREN test RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE" == rule){
    theCode += test(ptok->getKid()[2]);
    string stm2Start = getLabel();
    string stm2End = getLabel();
    //have an empty line after beq, but its okay
    theCode += ("beq $3, $0, " + stm2Start + "\n");
    
    theCode += statements(ptok->getKid()[5]);
    
    theCode += ("beq $0, $0, " +stm2End +"\n");
    theCode += (string(stm2Start) +":\n");
    theCode += statements(ptok->getKid()[9]);
    theCode += (string(stm2End) +":\n");
  }

  else if("statement WHILE LPAREN test RPAREN LBRACE statements RBRACE" == rule){
    string whileStart = getLabel();
    string whileEnd = getLabel();

    theCode += (string(whileStart) +":\n");
    theCode += test(ptok->getKid()[2]);
    //have an empty line after beq, but its okay
    theCode += ("beq $3, $0, " + whileEnd + "\n");
    
    theCode += statements(ptok->getKid()[5]);
    
    theCode += ("beq $0, $0, " +whileStart +"\n");
  
    theCode += (string(whileEnd) +":\n");
  }
 
  else if("statement PRINTLN LPAREN expr RPAREN SEMI" == rule){
    theCode += expr(ptok->getKid()[2]);
    //move value of $3 to $1
    theCode += move(1, 3, true);
    //call print
    theCode += "jalr $10\n";
  }
  
  else if("statement DELETE LBRACK RBRACK expr SEMI" == rule){
    theCode += expr(ptok->getKid()[3]);
    theCode += "beq $3, $11, 2\n";
    theCode += move(1, 3, true);
    theCode += "jalr $9\n";
  } 
  return theCode;
}

//use reg 12 for buffer
string code::test(token* ptok){
  string theCode = "; code for test\n";
  string rule = (ptok->getRule());
  theCode += (" ; the rule: " + string(rule) + "\n");

  theCode += expr(ptok->getKid()[0]);
  //move $3 to $12
  theCode += move(12,3,true);

  //push $3 on stack
  //theCode += (string("sw $3, -4($30)\n") + string("sub $30, $30, $4\n"));
  theCode += expr(ptok->getKid()[2]);
  //load stack to $5
  //theCode += (string("add $30, $30, $4\n") + string("lw $5, -4($30)\n"));

  //$3 = 1 if $12 != $3
  if("test expr NE expr" == rule){
    theCode += "bne $3, $12, 2\n";
    theCode += "add $3, $0, $0\n";
    theCode += "beq $0, $0, 1\n";
    theCode += "add $3, $0, $11\n";
  }
  //$3 = 1 if $12 < $3
  else if("test expr LT expr" == rule){
    theCode += "slt $3, $12, $3\n";
  }

  //$3 = 1 if $12 <= $3 (!$3 < $12)
  else if("test expr LE expr" == rule){
    theCode += "slt $3, $3, $12\n";
    theCode += "sub $3, $11, $3\n";
  }
  //$3 = 1 if $12 >= $3; !($12 < $3)
  else if("test expr GE expr" == rule){
    theCode += "slt $3, $12, $3\n";
    theCode += "sub $3, $11, $3\n";
  }
  //$3 = 1 if $12 > $3
  else if("test expr GT expr" == rule){
    theCode += "slt $3, $3, $12\n";
  
  }
  //$3 = 1 if $12 = $3
  else { //equal
    theCode += "beq $3, $12, 2\n";
    theCode += "add $3, $0, $0\n";
    theCode += "beq $0, $0, 1\n";
    theCode += "add $3, $0, $11\n";
  }
  return theCode;
}


//expr uses reg20 for tmp value
string code::expr(token* ptok){
  string theCode = "; code for expr\n";
  string rule = (ptok->getRule());
  theCode += (" ; the rule: " + string(rule) +"\n");
  //cout << "call expr, rule is: " << rule << endl;

  if("expr term" == rule){
    theCode += term(ptok->getKid()[0]);
    return theCode;
  }

  //question?? arithmetic on nullptr
  theCode += expr(ptok->getKid()[0]);

  string termRule = ptok->getKid()[2]->getRule();
  string factorRule = ptok->getKid()[2]->getKid()[0]->getRule();

  if((termRule == "term factor") 
    && ("factor ID" == factorRule || 
        "factor NUM" == factorRule || 
        "factor NULL" == factorRule) ){    
    theCode += move(20, 3, true);
    theCode += term(ptok->getKid()[2]);
  }

  else{
    //push $3 on stack
    theCode += (string("sw $3, -4($30)\n") + string("sub $30, $30, $4\n"));
    theCode += term(ptok->getKid()[2]);
    //load stack to $5
    theCode += (string("add $30, $30, $4\n") + string("lw $20, -4($30)\n"));
  }

  string op1Type = ptok->getKid()[0]->getType();
  string op2Type = ptok->getKid()[2]->getType();
  //op1 $5, op2 $3
  if("expr expr PLUS term" == rule){
   // cout << "plus operation, " << op1Type << "+" << op2Type << endl;
    if(op1Type == "int*" 
      && op2Type == "int"){
      theCode += "add $3, $3, $3\n";
      theCode += "add $3, $3, $3\n";       
    }  
    else if(op1Type == "int"
      && op2Type == "int*"){
      theCode += "add $20, $20, $20\n";
      theCode += "add $20, $20, $20\n";  
    }
    theCode += "add $3, $3, $20\n";
  }

  //op1 $5, op2 $3
  else{ //expr - term
    if(op1Type == "int*" 
      && op2Type == "int"){
      theCode += "add $3, $3, $3\n";
      theCode += "add $3, $3, $3\n";       
    } 

    theCode += "sub $3, $20, $3\n";

    if(op1Type == "int*" 
      && op2Type == "int*"){ //divide 4
      theCode += "div $3, $4\n";
      theCode += "mflo $3\n";
    }
  }
  return theCode;
}



// [calc result in $3]
//term uses reg 21 for tmp value
string code::term(token* ptok){
  string theCode = "; code for term\n";
  string rule = (ptok->getRule());
  theCode += (" ; the rule: " + string(rule) + "\n");
  //cout << "call term, rule: " << rule << endl;

  if("term factor" == rule){
    theCode += factor(ptok->getKid()[0]);
    return theCode;
  }

  theCode += term(ptok->getKid()[0]);

  string factorRule = ptok->getKid()[2]->getRule();
  if("factor ID" == factorRule || 
     "factor NUM" == factorRule || 
     "factor NULL" == factorRule ){
    theCode += move(21, 3, true);
    theCode += factor(ptok->getKid()[2]);
  }

  else{
    //push $3 on stack
    theCode += (string("sw $3, -4($30)\n") + string("sub $30, $30, $4\n"));
    theCode += factor(ptok->getKid()[2]);
    //load stack to $5
    theCode += (string("add $30, $30, $4\n") + string("lw $21, -4($30)\n"));
  }
  
  string operation = ptok->getKid()[1]->getKind();
  //op1 $6, op2 $3
  if("STAR" == operation){
    theCode += "mult $3, $21\n";
    theCode += "mflo $3\n";
    return theCode;
  }
   
  theCode += "div $21, $3\n";
  if("SLASH" == operation){
    theCode += "mflo $3\n";
  }

  else{ // % modulus
    theCode += "mfhi $3\n";
  }

  return theCode;
}



//put value of ID/ NUM. ect at $3
string code::factor(token* ptok){
  string theCode = "; code for factor\n";
  string rule = (ptok->getRule());
  theCode += (" ; the rule: " + string(rule) + "\n");
  //ptok->getKid()[0];
  //cout << "then ";
  //ptok->getKid()[1];
  //cout << "call factor, rule: " << rule << endl;


  if("factor ID" == rule){
    //cout << "in ID\n";
    string varName = ptok->getKid()[0]->getLexeme();

    //get value(might in reg or offset) of var in $3 if int
    //get addr in $3 if int*
    if(st.isExist(varName)){
      int repInt = st.getRep(varName);
      string rep = to_string(repInt);
      string varType = st.getType(varName);

      if(allReg) theCode += ("add $3, $0, $" + rep +"\n"); ////reg issue 
      else theCode += ("lw $3, " + rep + "($29)\n");

      /*if(varType == "int"){
        if(allReg) theCode += ("add $3, $0, $" + rep +"\n"); ////reg issue 
        else theCode += ("lw $3, " + rep + "($29)\n");
      }
      else {
        theCode += move(5, repInt, false);
        theCode += "add $3, $5, $29\n";
      }*/
    }
  }

  else if("factor NUM" == rule){
    string num = ptok->getKid()[0]->getLexeme();
    theCode += "lis $3\n";
    theCode += (".word " + num + "\n");
  }

  else if("factor NULL" == rule){
    theCode += "add $3, $0, $11\n";    
  }

  else if("factor LPAREN expr RPAREN" == rule) {
    theCode += expr(ptok->getKid()[1]);
  }

  else if("factor AMP lvalue" == rule){
    //cout << "why\n";
    //cout << "in amp " << ptok->getKid().size() << endl;
    theCode += lvalue(ptok->getKid()[1]);
   // cout << "after amp\n";
  }

  else if("factor STAR factor" == rule){
    //cout << "in star\n";
    theCode += factor(ptok->getKid()[1]);
    theCode += ("lw $3, 0($3)\n");
   // cout << "end star\n";
  }
 
  else { // new
    //cout << "new\n";
    //[expr]
    theCode += expr(ptok->getKid()[3]);
    //setupt & call new
    theCode += move(1, 3, true);
    theCode += "jalr $8\n";
    //cout << " finish new\n";
    theCode += "bne $3, $0, 1\n";
    theCode += "add $3, $0, $11\n";
  }
  return theCode;
}


string code::lvalue(token* ptok){
  //cout << "call lvalue\n";
  string theCode = "; code for lvalue\n";
  string rule = (ptok->getRule());
  theCode += (" ; the rule: " + string(rule) + "\n");
  
  //cout <<"call lvalue, rule: " << rule << endl;
  // Lookup, get offset
  // [calc addr, offset + $29] in $3
  if("lvalue ID" == rule){//"lvalue ID" == rule){    
    string varName = ptok->getKid()[0]->getLexeme();

    //get value(might in reg or offset) of var in $3
    if(st.isExist(varName)){
      int repInt = st.getRep(varName);
      string rep = to_string(repInt);
      if(allReg) theCode += ("add $3, $0, $" + string(rep) + "\n");
      else {
        theCode += move(5, repInt, false);
        theCode += ("add $3, $29, $5\n");
      }
    }
  }

  // [factor] --> val in $3
  //but we want addr
  else if("lvalue STAR factor" == rule){
    string factorRule = ptok->getKid()[1]->getRule();
    theCode += factor(ptok->getKid()[1]);
    //theCode += ("lw $3, 0($3)\n");
    //cout << "finish lvalue STAR factor\n";
  }

  else{ //(lvalue)
    theCode += lvalue(ptok->getKid()[1]);
  }
  return theCode;
}


string code::getLabel(){ //print a able output: "label"
  return ("label" + to_string(labelCounter++));
}



//code for move a val/reg to target reg
//isReg, if 2nd para is reg
//move val to reg if false
//move reg to targetReg if true
string code::move(int targetReg, int val, bool isReg){ //move val to reg
  string theCode = ""; // = "; move val to reg";  
  
  string target = to_string(targetReg);
  string value = to_string(val);
  if(isReg){
    theCode += ("add $" + string(target) + ", $" + string(value) + ", $0\n"); 
  }
  else{
    theCode += ("lis $" + string(target) + "\n");
    theCode += (".word " + string(value) + "\n");
  }
  return theCode;
}
//code for store reg at the offset wrt to $29
//sw $reg, offset($29)
string code::storeAtOffset(int reg, int offset){
  string theCode = ("sw $" + to_string(reg) + ", " + to_string(offset) + "($29)\n");
  return theCode;
}


//code to store the value(not reg) 
//at representation(reg or offset) -> checked internally
//for dcls or assignment
//bool is true if value (first para is a reg)
//make that offset store value
string code::storeAtRep(int value, int rep, bool isReg){  
  string theCode = "";
  //rep is reg
  if(allReg) theCode += move(rep, value, isReg);
  else{ //rep is offset
    //put value at $1 if value is not reg
    if(! isReg) {
      theCode += move(1, value, false);
      //store $1 at offset
      theCode += storeAtOffset(1, rep);
    }
    else{
      theCode += storeAtOffset(value, rep);    
    }  
    
  }
  return theCode;
}

//output string version of int
// string code::intTOstring(int num){
//   ostringstream os;
//   os << num;
//   string result = os.str(); 
//   return result;
// }


code::code(symbolTable& st, paTree& pa, ostream& codeStream):
  st{st}, pa{pa}, codeStream{codeStream}, labelCounter{0}{
  allReg = st.isReg();
  numOfVar = st.getNumOfVar();
}

void code::fullCode(){
  codeStream << prologue() << getCode(pa.patree) << epilogue();
  return;
}


