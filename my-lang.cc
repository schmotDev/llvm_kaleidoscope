#include <string>



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
  // we start a loop, first adding the lñast char to our string
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
