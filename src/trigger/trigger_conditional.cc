/*
 * Copyright (C) 2002-2008 by the Widelands Development Team
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


#include "trigger_conditional.h"

#include "compile_assert.h"
#include "events/event_chain.h"
#include "trigger.h"

#include "log.h"

#include "container_iterate.h"

namespace Widelands {

const char * const TriggerConditional_Factory::operators[] =
{"NOT", "AND", "OR", "XOR", "(", ")", "trigger"};

/**
 * Creates trigger conditionals, throws SyntaxError on error
 * This basically only converts the infix into postfix notation
 */
TriggerConditional & TriggerConditional_Factory::create_from_infix
	(EventChain & evchain, std::vector<Token> const & vec)
{
	std::vector<Token> tempstack;
	std::vector<Token> postfix;

	container_iterate_const(std::vector<Token>, vec, i)
		switch (TokenNames const token = i.current->token) {
		case LPAREN:
			tempstack.push_back(*i.current);
			break;
		case RPAREN: // append everything to our postfix notation
			for (;;) {
				if (!tempstack.size()) { // Mismatched parathesis
					ALIVE();
					log("Missmatched parenthesis!\n");
					throw SyntaxError();
				}
				if (tempstack.back().token == LPAREN) break;
				postfix.push_back(tempstack.back());
				tempstack.pop_back();
			}
			tempstack.pop_back(); // Pop the last left paranthesis
			break;
		case TRIGGER:
			postfix.push_back(*i.current);
			break;
		case NOT: case AND: case OR: case XOR:
			compile_assert(NOT < AND); //  Ensure proper operator precedence.
			compile_assert(AND < OR);  //
			compile_assert(OR  < XOR); //
			while (tempstack.size() and token > tempstack.back().token) {
				assert(tempstack.back().token < TRIGGER); //  Is operator
				postfix.push_back(tempstack.back());
				tempstack.pop_back();
			}
			tempstack.push_back(*i.current);
			break;
		default:
			assert(false);
		}

	//  Unload all operators which are left on stack.
	while (tempstack.size()) {
		if (tempstack.back().token == LPAREN) {
			ALIVE();
			log("Unmatched parenthesis!\n");
			throw SyntaxError();
		}
		postfix.push_back(tempstack.back());
		tempstack.pop_back();
	}

	return create_from_postfix(evchain, postfix);
}

/**
 * This effectivly creates a trigger conditional
 * out of a token list.
 *
 * Note that this function expects valid syntax.
 */
TriggerConditional & TriggerConditional_Factory::create_from_postfix
	(EventChain & evchain, std::vector<Token> const & vec)
{
	std::vector<TriggerConditional *> stk;
	container_iterate_const(std::vector<Token>, vec, i)
		switch (TokenNames const token = i.current->token) {
		case NOT: {
			assert(stk.size());
			TriggerConditional * & back = stk.back();
			back = new TriggerNOT(*back);
			break;
		}
		case AND: case OR: case XOR: {
			assert(stk.size() >= 2);
			TriggerConditional * r = stk.back();
			stk.pop_back();
			TriggerConditional * & l = stk.back();
			if      (token == AND) l = new TriggerAND(*l, *r);
			else if (token == OR)  l = new TriggerOR (*l, *r);
			else                   l = new TriggerXOR(*l, *r);
			break;
		}
		case TRIGGER: {
			Trigger & trigger = *i.current->data;
			trigger.reference(evchain);
			stk.push_back(new TriggerConditional_Var (trigger));
			break;
		}
		default:
			assert(false);
		}
	assert(stk.size() == 1);
	return *stk.back();
}


/**
 * TriggerConditional_Unary
 */
TriggerConditional_Unary ::TriggerConditional_Unary
	(TriggerConditional & cond)
	: m_conditional(cond)
{}
TriggerConditional_Unary ::~TriggerConditional_Unary () {
	delete &m_conditional;
}


void TriggerConditional_Unary ::get_infix_tokenlist
	(TriggerConditional::token_vector           & result,
	 TriggerConditional_Factory::TokenNames const outer_precedence)
	const
{
	bool const parentheses = outer_precedence < token();
	if (parentheses)
		result.push_back(TriggerConditional_Factory::LPAREN);
	result.push_back(token());
	m_conditional.get_infix_tokenlist(result, token());
	if (parentheses)
		result.push_back(TriggerConditional_Factory::RPAREN);
}
void TriggerConditional_Unary ::unreference_triggers
	(EventChain const & evch) const
{
	m_conditional.unreference_triggers(evch);
}
void TriggerConditional_Unary ::reset_triggers      (Game const & game) const {
	m_conditional.reset_triggers      (game);
}
bool TriggerConditional_Unary ::eval                (Game const & game) const {
	return do_eval(m_conditional.eval (game));
}


/**
 * TriggerConditional_Var
 */
TriggerConditional_Var ::TriggerConditional_Var (Trigger & trig)
	: m_trigger(trig)
{}
void TriggerConditional_Var ::get_infix_tokenlist
	(TriggerConditional::token_vector           & result,
	 TriggerConditional_Factory::TokenNames)
	const
{
	result.push_back
		(TriggerConditional_Factory::Token
		 	(TriggerConditional_Factory::TRIGGER, &m_trigger));
}
void TriggerConditional_Var   ::unreference_triggers
	(EventChain const & evch) const
{
	m_trigger.unreference         (evch);
}
void TriggerConditional_Var   ::reset_triggers      (Game const & game) const {
	m_trigger.reset_trigger       (game);
}
bool TriggerConditional_Var   ::eval                (Game const & game) const {
	m_trigger.check_set_conditions(game);
	return m_trigger.is_set();
}


/**
 * TriggerConditional_Binary
 */
TriggerConditional_Binary::TriggerConditional_Binary
	(TriggerConditional & l, TriggerConditional & r)
	: m_lconditional(l), m_rconditional(r)
{}
TriggerConditional_Binary::~TriggerConditional_Binary() {
	delete &m_lconditional;
	delete &m_rconditional;
}
void TriggerConditional_Binary::get_infix_tokenlist
	(TriggerConditional::token_vector           & result,
	 TriggerConditional_Factory::TokenNames const outer_precedence)
	const
{
	bool const parentheses = outer_precedence < token();
	if (parentheses)
		result.push_back(TriggerConditional_Factory::LPAREN);
	m_lconditional.get_infix_tokenlist(result, token());
	result.push_back(token());
	m_rconditional.get_infix_tokenlist(result, token());
	if (parentheses)
		result.push_back(TriggerConditional_Factory::RPAREN);
}
void TriggerConditional_Binary::unreference_triggers
	(EventChain const & evch) const
{
	m_lconditional.unreference_triggers(evch);
	m_rconditional.unreference_triggers(evch);
}
void TriggerConditional_Binary::reset_triggers      (Game const & game) const {
	m_lconditional.reset_triggers      (game);
	m_rconditional.reset_triggers      (game);
}
bool TriggerConditional_Binary::eval                (Game const & game) const {
	return do_eval(m_lconditional.eval(game), m_rconditional.eval(game));
}

/**
 * The effective Trigger Conditionals
 */
TriggerAND::TriggerAND(TriggerConditional & l, TriggerConditional & r)
	: TriggerConditional_Binary(l, r)
{}
bool TriggerAND::do_eval(bool const t1, bool const t2) const {
	return t1 && t2;
}

TriggerOR::TriggerOR (TriggerConditional & l, TriggerConditional & r)
	: TriggerConditional_Binary(l, r)
{}
bool TriggerOR ::do_eval(bool const t1, bool const t2) const {
	return t1 || t2;
}

TriggerXOR::TriggerXOR(TriggerConditional & l, TriggerConditional & r)
	: TriggerConditional_Binary(l, r)
{}
bool TriggerXOR::do_eval(bool const t1, bool const t2) const {
	return (t1 && !t2) || (!t1 && t2);
}

TriggerNOT::TriggerNOT(TriggerConditional & cond)
	: TriggerConditional_Unary (cond)
{}
bool TriggerNOT::do_eval(bool const t) const {
	return !t;
}

};
