










// TODO: Add the comment header here :)
// TODO: Create the load/save code, networkcode isn't needed (I think)



#include "battle.h"
#include "error.h"
#include "game.h"

/*class Battle : public BaseImmovable
{
   public:
         Battle();
         ~Battle();
      void init (Game*, Soldier*, Soldier*);
      virtual void act (Game*, uint);
      
   private:
      Soldier* first;
      Soldier* second;
};*/

class Battle_Descr : public Map_Object_Descr
{
   public:
      Battle_Descr() { }
      ~Battle_Descr() { }
};

Battle_Descr g_Battle_Descr;


Battle::Battle () : BaseImmovable(&g_Battle_Descr)
{
   m_first = 0;
   m_second = 0;
   m_last_try = 0;
}

Battle::~Battle ()
{
   if (m_first) log ("Battle : first wasn't removed yet!\n");
   if (m_second) log ("Battle : second wasn't removed yet!\n");
}

void Battle::init (Editor_Game_Base* eg, Soldier* s1, Soldier* s2)
{
   assert (eg);
   assert (s1);
   assert (s2);
   
   log ("Battle::init\n");
   Map_Object::init(eg);
   m_first = s1;
   m_second = s2;
   schedule_act ((Game*)eg, 1000); // Every round is 1000 ms
}

void Battle::cleanup (Editor_Game_Base* eg)
{
   log ("Battle::cleanup\n");
   m_first = 0;
   m_second = 0;
   Map_Object::cleanup(eg); 
}

void Battle::act (Game* g, uint data)
{
   log ("Battle::act\n");
   
   if (!g->is_game())
      return;
   
   Soldier* attacker;
   Soldier* defender;

   m_last_try = !m_last_try;
   if (m_last_try)
   {
      attacker = m_first;
      defender = m_second;
   }
   else
   {
      attacker = m_second;
      defender = m_first;
   }

   if (attacker->get_current_hitpoints() < 1)
   {
      attacker->send_signal(g, "die");
      defender->send_signal(g, "end_combat");
      m_first = 0;
      m_second = 0;
      schedule_destroy (g);
      return;
   }
   if (defender->get_current_hitpoints() < 1)
   {
      defender->send_signal(g, "die");
      attacker->send_signal(g, "end_combat");
      m_first = 0;
      m_second = 0;
      schedule_destroy (g);
      return;
   }
   
   // Put attack animation
   //attacker->start_animation(g, "attack", 1000);
   uint hit = g->logic_rand() % 100;
log (" hit=%d ", hit);
   if (hit > defender->get_evade())
   {
      uint attack = attacker->get_min_attack() + 
            (g->logic_rand()% (attacker->get_max_attack()-attacker->get_min_attack()-1));
      uint defend = defender->get_defense();
      defend = (attack * defend) / 100;

log (" attack(%d)=%d ", attacker->get_serial(), attack);
log (" defense(%d)=%d ", defender->get_serial(), defend);
log (" damage=%d\n", attack-defend);
      
      defender->damage (attack-defend);
      // defender->start_animation(g, "defend", 1000);
   }
   else
   {
log (" evade(%d)=%d\n", defender->get_serial(), defender->get_evade());
      //defender->start_animation(g, "evade", 1000);
   }
   schedule_act(g, 1000);
}


