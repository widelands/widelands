/*
 * Copyright (C) 2002-2006 by the Widelands Development Team
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
struct TriggerConditional_Factory {
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
	static const char * const operators[];
      struct Token {
         TokenNames token;
         void* data;
		};

      class SyntaxError {};

public:
      static TriggerConditional* create_from_infix(EventChain* evchain, const std::vector<Token>&);
      static TriggerConditional* create_from_postfix(EventChain* evchain, const std::vector<Token>&);
      static std::vector<Token>* create_tokenlist(const TriggerConditional*);
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
      virtual bool eval(Game*) = 0;

      virtual std::vector< TriggerConditional_Factory::Token >* get_infix_tokenlist() = 0;

      void unreference_triggers(EventChain*);
      void reset_triggers(Game*);
};

/*
 * A trigger conditional which takes only one argument
 */
struct TriggerConditional_OneArg : public TriggerConditional {
      TriggerConditional_OneArg(TriggerConditional*);
      virtual ~TriggerConditional_OneArg();

      virtual bool eval(Game*);
      virtual bool do_eval(bool) = 0;
      virtual std::vector< TriggerConditional_Factory::Token >* get_infix_tokenlist() ;

protected:
      virtual TriggerConditional_Factory::TokenNames get_token() = 0;

private:
      TriggerConditional *m_conditional;

};

struct TriggerConditional_Const : public TriggerConditional {
      TriggerConditional_Const(Trigger*);
      virtual bool eval(Game*);
      virtual std::vector< TriggerConditional_Factory::Token >* get_infix_tokenlist() ;

private:
      Trigger*   m_trigger;
};



/*
 * A trigger conditional which takes two arguments
 */
struct TriggerConditional_TwoArg : public TriggerConditional {
      TriggerConditional_TwoArg(TriggerConditional*, TriggerConditional*);
      virtual ~TriggerConditional_TwoArg();

      virtual bool eval(Game*);
      virtual bool do_eval(bool, bool) = 0;
      virtual std::vector< TriggerConditional_Factory::Token >* get_infix_tokenlist() ;

protected:
      virtual TriggerConditional_Factory::TokenNames get_token() = 0;

private:
      TriggerConditional *m_lconditional;
      TriggerConditional *m_rconditional;
};

/*
 * Now the effective TriggerConditionals
 */
struct TriggerAND : public TriggerConditional_TwoArg {
      TriggerAND(TriggerConditional*, TriggerConditional*);

      virtual bool do_eval(bool, bool);

protected:
      virtual TriggerConditional_Factory::TokenNames get_token() {return TriggerConditional_Factory::AND;}
};

struct TriggerOR : public TriggerConditional_TwoArg {
      TriggerOR(TriggerConditional*, TriggerConditional*);

      virtual bool do_eval(bool, bool);

protected:
      virtual TriggerConditional_Factory::TokenNames get_token() {return TriggerConditional_Factory::OR;}
};

struct TriggerXOR : public TriggerConditional_TwoArg {
      TriggerXOR(TriggerConditional*, TriggerConditional*);

      virtual bool do_eval(bool, bool);

protected:
      virtual TriggerConditional_Factory::TokenNames get_token() {return TriggerConditional_Factory::XOR;}
};

struct TriggerNOT : public TriggerConditional_OneArg {
      TriggerNOT(TriggerConditional*);

      virtual bool do_eval(bool);

protected:
      virtual TriggerConditional_Factory::TokenNames get_token() {return TriggerConditional_Factory::NOT;}
};

#endif // __S__TRIGGER_CONDITIONAL_H
