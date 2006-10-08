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
class TriggerConditional_Factory {
   public:
      // Note: these are ordered for easy precendence parsing
      enum TokenNames {
         TRIGGER = 0,  // not an operator
         OPERATOR = 1, // Everything over this is an operator
         RPAREN = 11,
         LPAREN = 12,
         XOR = 40,
         OR = 50,
         AND = 60,
         NOT = 70,
      };
      struct Token {
         TokenNames token;
         void* data;
      };

      class SyntaxError {};

   public:
      static TriggerConditional* create_from_infix( EventChain* evchain, const std::vector<Token>& );
      static TriggerConditional* create_from_postfix( EventChain* evchain, const std::vector<Token>& );
      static std::vector<Token>* create_tokenlist( const TriggerConditional* );
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
 *  - 2 conditionals or triggers (OR,AND,XOR)
 *  - 1 conditional AND 1 trigger (OR,AND,XOR)
 */
class TriggerConditional {
   public:
      virtual ~TriggerConditional( void ) { };
      virtual bool eval( Game* ) = 0;

      virtual std::vector< TriggerConditional_Factory::Token >* get_infix_tokenlist( void ) = 0;

      void unreference_triggers( EventChain* );
      void reset_triggers( Game* );
};

/*
 * A trigger conditional which takes only one argument
 */
class TriggerConditional_OneArg : public TriggerConditional {
   public:
      TriggerConditional_OneArg(TriggerConditional*);
      virtual ~TriggerConditional_OneArg( void );

      virtual bool eval( Game* );
      virtual bool do_eval( bool ) = 0;
      virtual std::vector< TriggerConditional_Factory::Token >* get_infix_tokenlist( void ) ;

   protected:
      virtual TriggerConditional_Factory::TokenNames get_token( void ) = 0;

   private:
      TriggerConditional *m_conditional;

};

class TriggerConditional_Const : public TriggerConditional {
   public:
      TriggerConditional_Const(Trigger*);
      virtual bool eval( Game* );
      virtual std::vector< TriggerConditional_Factory::Token >* get_infix_tokenlist( void ) ;

   private:
      Trigger*   m_trigger;
};



/*
 * A trigger conditional which takes two arguments
 */
class TriggerConditional_TwoArg : public TriggerConditional {
   public:
      TriggerConditional_TwoArg(TriggerConditional*, TriggerConditional*);
      virtual ~TriggerConditional_TwoArg( void );

      virtual bool eval( Game* );
      virtual bool do_eval( bool, bool) = 0;
      virtual std::vector< TriggerConditional_Factory::Token >* get_infix_tokenlist( void ) ;

   protected:
      virtual TriggerConditional_Factory::TokenNames get_token( void ) = 0;

   private:
      TriggerConditional *m_lconditional;
      TriggerConditional *m_rconditional;
};

/*
 * Now the effective TriggerConditionals
 */
class TriggerAND : public TriggerConditional_TwoArg {
   public:
      TriggerAND(TriggerConditional*, TriggerConditional*);

      virtual bool do_eval( bool, bool);

   protected:
      virtual TriggerConditional_Factory::TokenNames get_token( void ) { return TriggerConditional_Factory::AND; }
};

class TriggerOR : public TriggerConditional_TwoArg {
   public:
      TriggerOR(TriggerConditional*, TriggerConditional*);

      virtual bool do_eval( bool, bool);

   protected:
      virtual TriggerConditional_Factory::TokenNames get_token( void ) { return TriggerConditional_Factory::OR; }
};

class TriggerXOR : public TriggerConditional_TwoArg {
   public:
      TriggerXOR(TriggerConditional*, TriggerConditional*);

      virtual bool do_eval( bool, bool);

   protected:
      virtual TriggerConditional_Factory::TokenNames get_token( void ) { return TriggerConditional_Factory::XOR; }
};

class TriggerNOT : public TriggerConditional_OneArg {
   public:
      TriggerNOT(TriggerConditional*);

      virtual bool do_eval( bool );

   protected:
      virtual TriggerConditional_Factory::TokenNames get_token( void ) { return TriggerConditional_Factory::NOT; }
};

#endif // __S__TRIGGER_CONDITIONAL_H
