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

#ifndef __S__PARSER_H
#define __S__PARSER_H

#include "mytypes.h"

#define MAX_OPT_LENGTH	255
#define MAX_OPTS		100
/** class Parser
 *
 * This is a simple Text parser. you register keywords, what kind
 * of vars this will be and the variable that should be registered
 * with it
 */
class Parser {
		  public:
					 Parser();
					 ~Parser();

					 void register_string_opt(const char*, char*);
					 void register_int_opt(const char*, int*);
					 void register_bool_opt(const char*, bool*);

					 int parse_line(const char*);					 
					 

		  private:
					
					 
					 enum Type {
										  T_BOOL,
										  T_STRING,
										  T_INT
								};
 
					 struct {
								char opt[MAX_OPT_LENGTH];
								void* var;
								Type type;
					 } opts[MAX_OPTS];
					 
					 uint nopts;

					 /* don't call this functions */
					 Parser(const Parser&);
					 const Parser& operator=(const Parser&);
};



#endif /* __S__PARSER_H */
