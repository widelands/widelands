-- =======================================================================
--                       Barbarians Campaign Mission 1
-- =======================================================================

set_textdomain("campaigns/t01.wmf")

-- ===============
-- Initialization
-- ===============

p = wl.game.Player(1)

-- Only lumberjack buildings are allowed
p:forbid_buildings("all")
p:allow_buildings{"lumberjacks_hut"}

-- Place the headquarters & fill it with wares
hq = p:place_building("headquarters_interim", wl.map.Field(12,10))
hq:set_wares{
   trunk = 80
}
hq:set_workers{
   builder=10,
   carrier=40,
   lumberjack=3,
   miner=4,
   ranger=1,
   stonemason=2
}

-- ==========
-- Constants 
-- ==========
home = wl.map.Field(12,10)
al_thunran = wl.map.Field(53, 43)
grave = wl.map.Field(25,22)

-- This function can move to a place, display a modal message box and return
function show_story_box(t, m, pos, gposx, gposy)
   plr = wl.game.Player(1)
   posx = gposx
   posy = gposy
   if pos ~= nil then
      plr:reveal_fields(pos:region(8))
      smooth_move(pos)
      sleep(1200)
      posx = gposx or 0
      posy = gposy or 80
   end
   plr:message_box(t, m, { posx = posx, posy = posy })
   sleep(500)
end

use("aux", "coroutine")
use("aux", "smooth_move")

use("map", "texts")

use("map", "initial_messages")
use("map", "story_messages")

