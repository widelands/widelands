













#include "immovable.h"
#include "soldier.h"


class Battle : public BaseImmovable
{
   public:
         Battle();
         ~Battle();
      virtual int get_type () { return IMMOVABLE; }
      virtual int get_size () { return SMALL; }
      virtual bool get_passable () { return false; }
      virtual void draw (Editor_Game_Base* eg, RenderTarget* rt, FCoords f, Point p) { }
      
      void init (Editor_Game_Base*, Soldier*, Soldier*);
      virtual void cleanup (Editor_Game_Base*);
      virtual void act (Game*, uint);
      
   private:
      Soldier* m_first;
      Soldier* m_second;
      int      m_last_try;
};







