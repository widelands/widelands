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

use("map", "initial_messages")

