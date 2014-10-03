-- =======================================================================
--                       Barbarians campaign mission 1
-- =======================================================================

set_textdomain("scenario_t01.wmf")

-- ===============
-- Initialization
-- ===============

-- Only lumberjack buildings are allowed
p:forbid_buildings("all")
p:allow_buildings{"lumberjacks_hut"}

-- Place the headquarters & fill it with wares
hq = p:place_building("headquarters_interim", wl.Game().map:get_field(12,10),
   false, true)
hq:set_wares{
   log = 80
}
hq:set_workers{
   builder=10,
   carrier=40,
   lumberjack=3,
   miner=4,
   ranger=1,
   stonemason=2
}
