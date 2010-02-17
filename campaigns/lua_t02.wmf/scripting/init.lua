-- =======================================================================
--                       Barbarians Campaign Mission 2                      
-- =======================================================================

set_textdomain("campaigns/t02.wmf")

use("aux", "coroutine")

-- ===============
-- Initialization 
-- ===============

p = wl.game.Player(1)

-- Allow some buildings
p:forbid_buildings("all")
p:allow_buildings{
   "lumberjacks_hut",
   "quarry", 
   "rangers_hut", 
   "sentry",
   "stronghold"
}

-- Place hq and fill it with wares
hq = p:place_building("headquarters_interim", wl.map.Field(12,10))
hq:set_wares{
   axe=6,
   bakingtray=2,
   blackwood=32,
   coal=12,
   fire_tongs=2,
   fish=6,
   grout=12,
   hammer=12,
   hunting_spear=2,
   iron=12,
   ironore=5,
   kitchen_tools=4,
   meal=4,
   meat=6,
   pick=14,
   pittabread=8,
   ration=12,
   raw_stone=40,
   scythe=6,
   shovel=4,
   snack=3,
   thatchreed=24,
   trunk=80,
}
hq:set_workers{
   blacksmith=2,
   brewer=1,
   builder=10,
   burner=1,
   carrier=40,
   ferner=1,
   geologist=4,
   ["lime-burner"]=1,
   lumberjack=3,
   miner=4,
   ranger=1,
   stonemason=2
}
-- TODO: soldiers. 0/0/0/0=45
-- TODO: roads

-- Place some buildings from the last map
-- TODO: add workers to these buildings
p:place_building("lumberjacks_hut", wl.map.Field(15,11))
p:place_building("lumberjacks_hut", wl.map.Field(16,12))
p:place_building("quarry", wl.map.Field(8,12))
p:place_building("rangers_hut", wl.map.Field(9,13))

use("map", "mission_thread")

