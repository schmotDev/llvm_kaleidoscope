#include <string>
#include <iostream>

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>
using namespace std;




//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//


// The lexer returns tokens [0-255] if it is an unknown character, otherwise one
// of these for known things.
enum Token {
  tok_eof = -1,

  // commands
  tok_def = -2,
  tok_extern = -3,

  // primary
  tok_identifier = -4,
  tok_number = -5
};

static std::string IdentifierStr; // will store value of identifier if tok_identifier has been found by lexer
static double NumVal;             // will store value of number if tok_number has been found by lexer


/// gettok - Return the next token from standard input.
// the function first read from standard input
// then it checks the type of char being read
// and we read the next char to try to identify a token

// space = we skip
// alpha = we check if it's a known word, otherwise it's a variable name
// digit = we keep the value
// #     = it's a comment
// EOF   = we stop reading

// in each check function, we move to the next char with a call to getchar()
// we call getchar(), check the value, and if the char is part of the token we loop
// if niot we just go out, but we are postionned on the next char
// at the end we make a single call to getchar() before we return the operator


static int gettok() {
  static int LastChar = ' ';

  // Skip any whitespace.
  while (isspace(LastChar)) {
    LastChar = getchar();
  }


  // if the char is alpha, we read the next one
  // we continue while we find alphanum
  // so we respect the rule definition of identifier   [a-zA-Z][a-zA-Z0-9]*
  // we check if the token is "known" otherwise we return return it's an identifier
  // identifierStr stores the value (name)

  if (isalpha(LastChar)) {
    IdentifierStr = LastChar;
    while (isalnum((LastChar = getchar()))) {
      IdentifierStr += LastChar;
    }

    if (IdentifierStr == "def")
      return tok_def;
    if (IdentifierStr == "extern")
      return tok_extern;
    return tok_identifier;
  }

  // if the first char is a digit or a ".",
  // we initialize a string
  // we start a loop, first adding the last char to our string
  // then we read the next char
  // while we find a digit or a "." we loop again
  // NOTE: this will accept entries such as 12.222.34.23
  // finally we convert our string to double and return the value 

  if (isdigit(LastChar) || LastChar == '.') {   // Number: [0-9.]+
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), 0);
    return tok_number;
  }


  // if the char is a # then we found a comment
  // we read the next char and loop while we don't find EOF, EOL or CR
  // after the loop, is the char is not EOF, we call recursively the function
  // as we just skipped what was read, we start again
  if (LastChar == '#') {
    // Comment until end of line.
    do
      LastChar = getchar();
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return gettok();
  }

  // if we reach this, we didn't find anything so it's either an EOF or an operatoir such as '+'

  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF)
    return tok_eof;

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}

//===----------------------------------------------------------------------===//
// End of the Lexer
//===----------------------------------------------------------------------===//



//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//


// The AST is the ouput of the Parser
// The AST for a given program to be analyzed, will capture its behaviour
// so it can be easily interpreted (by the compiler)
// We want one object for each of the construct of the language
// In Kaleidoscope, we have expressions, a prototype, and a function object

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
  virtual ~ExprAST() = default;
};


/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
  double Val;

public:
  NumberExprAST(double paramVal) : Val(paramVal) {}
};
// *******   C++ specific:   ***************
// ": Val(paramVal)" is equivalent to "this.Val = paramVal;" like in other languages


/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
  std::string Name;

public:
  VariableExprAST(const std::string &paramName) : Name(paramName) {}
};
// we use the reference of paramName = more efficient
// we declare as constant just to make sure we don't modify the value in this sub-function


// BinaryExprAST - Expression class for a binary operator.
// LHS = left hand side
// RHS = right hand side
class BinaryExprAST : public ExprAST {
  char Op; // can be + - * / < >
  std::unique_ptr<ExprAST> LHS;
  std::unique_ptr<ExprAST> RHS;

// ********* C++ specific:  ********************
// the std:unique_ptr is a smart pointer = no need to clean up with a destructor

public:
  BinaryExprAST(char param_Op, std::unique_ptr<ExprAST> param_LHS,
                std::unique_ptr<ExprAST> param_RHS)
    : Op(param_Op), LHS(std::move(param_LHS)), RHS(std::move(param_RHS)) {}
};

// ********* C++ specific:  ********************
// std:move(param) is taking a copy but also saying to compiler that the original can be destroyed
// it's more like swaping values than copying values
// we do this here because we don't need the original LHS and HDS anymore after getting the values


/// CallExprAST - Expression class for function calls.
// callee is the name of the function being called

class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  CallExprAST(const std::string &param_Callee,
              std::vector<std::unique_ptr<ExprAST>> param_Args)
    : Callee(param_Callee), Args(std::move(param_Args)) {}
};


/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;

public:
  PrototypeAST(const std::string &param_Name, std::vector<std::string> param_Args)
    : Name(param_Name), Args(std::move(param_Args)) {}

  const std::string &getName() const { return Name; }
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<ExprAST> Body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> param_Proto,
              std::unique_ptr<ExprAST> param_Body)
    : Proto(std::move(param_Proto)), Body(std::move(param_Body)) {}
};



//===----------------------------------------------------------------------===//
// END OF Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//



//===----------------------------------------------------------------------===//
// Parser ( will generate our Abstract Syntax Tree)
//===----------------------------------------------------------------------===//

/// CurTok/getNextToken - Provide a simple token buffer.  CurTok is the current
/// token the parser is looking at.  getNextToken reads another token from the
/// lexer and updates CurTok with its results.
// this way we can have a look at a token, and the next token ahead
static int CurTok;
static int getNextToken() { return CurTok = gettok(); }


// routines for error handling
/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}


//===----------------------------------------------------------------------===//
// basic expressions parsing

/// numberexpr ::= number
// will be called when current token is tok_number (-5)
// it takes the the current number value, creates a NumberExpreAST node,
// move the lexer to next token, and returns
static std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(NumVal);
  getNextToken(); // consume the number
  return std::move(Result);
}

// the routine "eats" all the of th etoken that correspond to the production
// and returns the lexer buffer with the next token (which is not part of this production)
// standard way of recursive descent parsers

/// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken(); // eat (.
  auto V = ParseExpression();
  if (!V)
    return nullptr;

  if (CurTok != ')')
    return LogError("expected ')'");
  getNextToken(); // eat ).
  return V;
}

// after we read '(' we call ParseExpression() = recursive
// ParseExpression will be able to call ParseParenExpr, etc etc
// this provide recursive grammar

// the '()' do not create AST nodes. the '()' only guide the parser providig grouping
// once the parser constructs the AST, '()' are not needed



// identifier can be varable name of function call
// we check if we find a '(' to make the difference
// constructing either a VariableExprAST or CallExprAST node as appropriate
// if we work on function call we need to read the args with the loop

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
  std::string IdName = IdentifierStr;

  getNextToken();  // eat identifier.

  if (CurTok != '(') // Simple variable ref.
    return std::make_unique<VariableExprAST>(IdName);

  // Call.
  getNextToken();  // eat (
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (CurTok != ')') {
    while (true) {
      if (auto Arg = ParseExpression())
        Args.push_back(std::move(Arg));
      else
        return nullptr;

      if (CurTok == ')')
        break;

      if (CurTok != ',')
        return LogError("Expected ')' or ',' in argument list");
      getNextToken();
    }
  }

  // Eat the ')'.
  getNextToken();

  return std::make_unique<CallExprAST>(IdName, std::move(Args));
}






// single main function
// we call gettok and print

int main() {
    while (true) {
        int tok = gettok();
        cout << "got token: " << tok << endl;
    }
}


