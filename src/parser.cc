/*
 * Copyright (C) 2002 by Holger Rapp 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include "parser.h"
#include "errors.h"
#include <stdlib.h>
#ifdef WIN32
#include <string.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Class Token
 *
 * This implements a simple token parser. 
 * it's working with expect and check
 *
 */
class Token {
		  public:
					 Token(void);
					 ~Token(void);

					 void set_line(const char*);

					 bool expect_identifier(char*) ;
					 bool expect_operator(void);
					 bool expect_int(int*);
					 bool expect_bool(bool*);
					 bool expect_string(char*);
					 bool	end_of_string(void);
					 
		  private:
					 void forward_to_next_char(void);
					 inline bool is_valid_char(char);
					 inline bool is_valid_num(char);
					 bool str_end;
					 const char* str;
					 int n;
					 int len;
};

/** Token::Token(void)
 *
 * Default constructor. Trivial inits
 *
 * Args: none
 * returns: Nothing
 */
Token::Token(void) {
		  str=0;
		  n=0;
		  str_end=0;
		  len=0;
}

/** Token::~Token(void) 
 *
 * Default Destructor. Clenaups
 *
 * Args: none
 * returns: Nothing
 */
Token::~Token(void) {
		  str=0; // Not freed, we don't own this memory
		  n=0;
}

/** void Token::set_line(const char* line) 
 *
 * Sets the line we currently try to parse
 *
 * Args: str	Line to parse
 * returns: Nothing
 */
void Token::set_line(const char* line) {
		  str=line;
		  str_end=0;
		  n=0;
		  len=strlen(line);
		  
		  forward_to_next_char();
}

/** bool Token::end_of_string(void) 
 *
 * Checks if this line is completly parsed
 *
 * Args: none
 * Returns: true if string is parsed to the end, false otherwise
 */
bool Token::end_of_string(void) {
		  return str_end;
}

/** bool Token::expect_identifier(char* ident)
 *
 * Tells the token to search for a identifier and to safe it
 *
 * Args: ident	safe indentifier in it
 * Returns: true if identifier is found, false otherwise
 */
bool Token::expect_identifier(char* ident) {
		  uint i=0;
		  
		  forward_to_next_char();
		  
		  while(is_valid_char(str[n])) { 
					 ident[i++]=str[n++];
		  }
		  ident[i]='\0';


		  forward_to_next_char();
		  if(!i) return false;
		  return true;
}

/** bool Token::expect_bool(bool* var)
 *
 * Tells the token to search for a var and to safe it
 *
 * Args: var	safe variable in it
 * Returns: true if matching var is found, false otherwise
 */
bool Token::expect_bool(bool* var) {
		  if(str[n]=='1') *var=1;
		  else if(str[n]=='0') *var=0;
		  
		  else if(str[n]=='y') *var=1;
		  else if(str[n]=='Y') *var=1;
		  else if(str[n]=='n') *var=0;
		  else if(str[n]=='N') *var=0;
		  
		  else if(str[n]=='t') *var=1;
		  else if(str[n]=='T') *var=1;
		  else if(str[n]=='f') *var=0;
		  else if(str[n]=='F') *var=0;
		  else { *var=0; return false; }

		  while(is_valid_char(str[n])) { 
					 n++;
		  }
		  forward_to_next_char();
		  return true;
}

/** bool Token::expect_int(int* var)
 *
 * Tells the token to search for a var and to safe it
 *
 * Args: var	safe variable in it
 * Returns: true if matching var is found, false otherwise
 */
bool Token::expect_int(int* var) {
		  char buf[20];
		  uint i=0;
		  
		  while(is_valid_num(str[n])) { 
					 buf[i++]=str[n++];
		  }
		  buf[i]='\0';

		  if(i) {
					 *var=atoi(buf);
		  }

		  forward_to_next_char();
		  if(!i) return false;
		  
		  forward_to_next_char();
		  return true;
}

/** bool Token::expect_string(char* var)
 *
 * Tells the token to search for a var and to safe it
 *
 * Args: var	safe variable in it
 * Returns: true if matching var is found, false otherwise
 */
bool Token::expect_string(char* var) {
		  uint i=0;
		  bool skip=0;

		  
		  if(str[n]=='"' || str[n]=='\'') { ++n ; skip=1; }
		  if(skip && ( str[n]=='"' || str[n]=='\'')) {
					 // was: "" 
					 var[0]='\0';
					 ++n;
					 forward_to_next_char();
					 return true;
		  }

		  while(is_valid_char(str[n])) { 
					 var[i++]=str[n++];
		  }
		  var[i]='\0';
		  
		  if(skip) ++n;

		  forward_to_next_char();
		  if(!i) return false;
		  return true;
}

/** bool Token::expect_operator(void)
 *
 * Tells the token to search for a operator
 *
 * Args: None
 * Returns: true if operator is found, false otherwise
 */
bool Token::expect_operator(void) {
		  bool retval=false;

		  if(str[n++] == '=') retval=true;
		 
		  forward_to_next_char();
		  return retval;
}


/** inline bool Token::is_valid_char(char c) 
 *
 * ************ PRIVATE FUNCTION ********************
 *
 * Checks if this is a valid char for a identifier (means, no whitespace, 
 * no operator)
 *
 * Args: c	Char to check
 * Returns: when char is valid true, else false
 */
inline bool Token::is_valid_char(char c) {
		  if(c=='\t') return false;
		  if(c=='\n') return false;
		  if(c=='\r') return false;
		  if(c=='\b') return false;
		  if(c==' ') return false;
		  if(c=='=') return false;
		  if(c=='"') return false;
		  if(c=='\'') return false;
		  if(c=='\0') return false;
		  if(c=='#') return false;
		  return true;
}

/** inline bool Token::is_valid_num(char c) 
 *
 * ************ PRIVATE FUNCTION ********************
 *
 * Checks if this is a valid char for a int number 
 *
 * Args: c	Char to check
 * Returns: when char is valid true, else false
 */
inline bool Token::is_valid_num(char c) {
		  if(!is_valid_char(c)) return false;
		  
		  if(c=='-') return true;
		  if(c=='0') return true;
		  if(c=='1') return true;
		  if(c=='2') return true;
		  if(c=='3') return true;
		  if(c=='4') return true;
		  if(c=='5') return true;
		  if(c=='6') return true;
		  if(c=='7') return true;
		  if(c=='8') return true;
		  if(c=='9') return true;
		  
		  return false;
}

/** void Token::forward_to_next_char(void) 
 *
 * ************ PRIVATE FUNCTION ********************
 *
 * Private function to set the pointer to the next char
 *
 * Args: none
 * returns: Nothing
 */
void Token::forward_to_next_char(void) {
		  if(n>=len) {
					 str_end=1;
					 return;
		  }
		  
		  while(!is_valid_char(str[n]) && str[n]!='=' && str[n]!='"' && str[n]!='\'' && str[n]!='#') n++;

		  if(n>=len) str_end=1;
		  if(str[n] == '#' || str[n] == '\0') str_end=1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Class Parser
 *
 * this implements the parser class. main class of file
 */

/** Parser::Parser()
 *
 * Default constructor. basic inits
 *
 * Args: none
 * returns: nothing
 * 
 */
Parser::Parser(void) {
		  nopts=0;
}

/** Parser::~Parser()
 *
 * Default destructor. Simple cleanups
 *
 * Args: none
 * returns: nothing
 * 
 */
Parser::~Parser(void) {
}

/** Parser::register_int_opt(const char* opt, int* var)
 *
 * Registers one option to find with a variable
 *
 * Args:	opt 	string which defines the variable
 * 		var	pointer to var in which to store the option
 */
void	Parser::register_int_opt(const char* opt, int* var) {
		  if(nopts >= MAX_OPTS) return;

		  strcpy(opts[nopts].opt, opt);
		  opts[nopts].var= var;
		  opts[nopts].type=T_INT;

		  ++nopts;
}

/** Parser::register_bool_opt(const char* opt, bool* var)
 *
 * Registers one option to find with a variable
 *
 * Args:	opt 	string which defines the variable
 * 		var	pointer to var in which to store the option
 */
void	Parser::register_bool_opt(const char* opt, bool* var) {
		  if(nopts >= MAX_OPTS) return;

		  strcpy(opts[nopts].opt, opt);
		  opts[nopts].var= var;
		  opts[nopts].type=T_BOOL;
		  
		  ++nopts;
}

/** Parser::register_string_opt(const char* opt, char* var)
 *
 * Registers one option to find with a variable
 *
 * Args:	opt 	string which defines the variable
 * 		var	pointer to var in which to store the option
 */
void	Parser::register_string_opt(const char* opt, char* var) {
		  if(nopts >= MAX_OPTS) return;

		  strcpy(opts[nopts].opt, opt);
		  opts[nopts].var= var;
		  opts[nopts].type=T_STRING;
		  
		  ++nopts;
}

/** int Parser::parse_line(const char* str) 
 *
 * Parses one line. sets the registers options accordingly
 *
 * Args: str   line to parse
 * Returns: ERR_FAILED on Error, RET_OK else
 */
int Parser::parse_line(const char* str) {
		  char opt[MAX_OPT_LENGTH];
		  Token p;
		  uint n;
		  
		  p.set_line(str);
		  while(!p.end_of_string()) {
					 if(!p.expect_identifier(opt)) { 
								return ERR_FAILED;
					 }
				 
					 for(n=0; n<nopts; n++) {
								if(!strcmp(opt, opts[n].opt)) break;
					 }
					 if(n==nopts) {
								return ERR_FAILED;
					 }


					 switch(opts[n].type) {
								case T_BOOL:
										  if(!p.expect_operator()) {
													 *((bool*) (opts[n].var))=true;
													 break;
										  } else if(!p.expect_bool((bool*) opts[n].var)) {
													 return ERR_FAILED;
										  }
										  break;

								case T_INT:
										  if(!p.expect_operator()) {
													 return ERR_FAILED;
										  }
										  if(!p.expect_int((int*) opts[n].var)) {
													 return ERR_FAILED;
										  }
										  break;

								case T_STRING:
										  if(!p.expect_operator()) {
													 return ERR_FAILED;
										  }
										  if(!p.expect_string((char*) opts[n].var)) {
													 return ERR_FAILED;
										  }
										  break;
					 }
					 
		  }
		  
		  return RET_OK;
}

