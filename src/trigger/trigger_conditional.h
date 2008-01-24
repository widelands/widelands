/*
 * Copyright (C) 2002-2006, 2008 by the Widelands Development Team
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

#ifndef __S__TRIGGER_CONDITIONAL_H
#define __S__TRIGGER_CONDITIONAL_H

#include <vector>

namespace Widelands {

class EventChain;
class Trigger;
class Game;

/*
 * This class creates TriggerConditionals
 * from various inputs.
 *
 * Either from a file (prefix notation)
 * or from a array of tokens.
 */
class TriggerConditional;
namespace TriggerConditional_Factory {
      // Note: these are ordered for easy precendence parsing
enum TokenNames {
	NOT,
	AND,
	OR,
	XOR,
	LPAREN,
	RPAREN,
	TRIGGER,  // not an operator
};
extern const char * const operators[];
struct Token {
	Token(TokenNames const T, Trigger * const D = 0) : token(T), data(D) {}
	TokenNames token;
	Trigger  * data;
};

struct SyntaxError {};

TriggerConditional & create_from_infix
	(EventChain &, std::vector<Token> const &);
TriggerConditional & create_from_postfix
	(EventChain &, std::vector<Token> const &);
std::vector<Token> & create_tokenlist(TriggerConditional const &);
};


/*
 * A trigger conditional is a combination of
 * bool conditionals like AND, OR, XOR or NOT
 *
 * It is used to check if all given triggers are
 * valid.
 *
 * Each conditional either works on the following:
 *  - 1 conditional or trigger (NOT)
 *  - 2 conditionals or triggers (OR, AND, XOR)
 *  - 1 conditional AND 1 trigger (OR, AND, XOR)
 */
struct TriggerConditional {
	virtual ~TriggerConditional() {};

	typedef std::vector<TriggerConditional_Factory::Token> token_vector;

	/// Adds tokens to the end of the vector. Will add parentheses around itself
	/// if it has lower precedence than outer_precedence.
	virtual void get_infix_tokenlist
		(token_vector &,
		 TriggerConditional_Factory::TokenNames outer_precedence =
		 TriggerConditional_Factory::TRIGGER)
		const
		= 0;

	virtual void unreference_triggers(EventChain const &) const = 0;
	virtual void reset_triggers      (Game       const &) const = 0;
	virtual bool eval                (Game       const &) const = 0;
};

/*
 * A trigger conditional which takes only one argument
 */
struct TriggerConditional_Unary  : public TriggerConditional {
	TriggerConditional_Unary (TriggerConditional &);
	~TriggerConditional_Unary ();

	void get_infix_tokenlist
		(token_vector &,
		 TriggerConditional_Factory::TokenNames outer_precedence)
		const;
	void unreference_triggers(EventChain const &) const;
	void reset_triggers      (Game       const &) const;
	bool eval                (Game       const &) const;
	virtual bool do_eval(bool) const = 0;

protected:
	virtual TriggerConditional_Factory::TokenNames token() const = 0;

private:
	TriggerConditional & m_conditional;
};

struct TriggerConditional_Var  : public TriggerConditional {
	TriggerConditional_Var (Trigger &);
	void get_infix_tokenlist
		(token_vector &,
		 TriggerConditional_Factory::TokenNames outer_precedence)
		const;
	void unreference_triggers(EventChain const &) const;
	void reset_triggers      (Game       const &) const;
	bool eval                (Game       const &) const;

private:
	Trigger & m_trigger;
};



/*
 * A trigger conditional which takes two arguments
 */
struct TriggerConditional_Binary : public TriggerConditional {
	TriggerConditional_Binary(TriggerConditional &, TriggerConditional &);
	virtual ~TriggerConditional_Binary();

	void get_infix_tokenlist
		(token_vector &,
		 TriggerConditional_Factory::TokenNames outer_precedence)
		const;
	void unreference_triggers(EventChain const &) const;
	void reset_triggers      (Game       const &) const;
	bool eval                (Game       const &) const;
	virtual bool do_eval(bool, bool) const = 0;

protected:
	virtual TriggerConditional_Factory::TokenNames token() const = 0;

private:
	TriggerConditional & m_lconditional;
	TriggerConditional & m_rconditional;
};

/*
 * Now the effective TriggerConditionals
 */
struct TriggerAND : public TriggerConditional_Binary {
	TriggerAND(TriggerConditional &, TriggerConditional &);

	bool do_eval(bool, bool) const;

protected:
	virtual TriggerConditional_Factory::TokenNames token() const {
		return TriggerConditional_Factory::AND;
	}
};

struct TriggerOR : public TriggerConditional_Binary {
	TriggerOR(TriggerConditional &, TriggerConditional &);

	bool do_eval(bool, bool) const;

protected:
	virtual TriggerConditional_Factory::TokenNames token() const {
		return TriggerConditional_Factory::OR;
	}
};

struct TriggerXOR : public TriggerConditional_Binary {
	TriggerXOR(TriggerConditional &, TriggerConditional &);

	bool do_eval(bool, bool) const;

protected:
	virtual TriggerConditional_Factory::TokenNames token() const {
		return TriggerConditional_Factory::XOR;
	}
};

struct TriggerNOT : public TriggerConditional_Unary  {
	TriggerNOT(TriggerConditional &);

	bool do_eval(bool) const;

protected:
	virtual TriggerConditional_Factory::TokenNames token() const {
		return TriggerConditional_Factory::NOT;
	}
};

};

#endif // __S__TRIGGER_CONDITIONAL_H
