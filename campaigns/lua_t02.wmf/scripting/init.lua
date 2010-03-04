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
hq_pos = wl.map.Field(12,10)
hq = p:place_building("headquarters_interim", hq_pos)
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
hq:set_soldiers({0,0,0,0}, 45)

-- ============
-- Build roads 
-- ============
function connected_road(p, start, cmd, g_create_carriers)
   create_carriers = g_create_carriers or true

   if cmd:sub(-1) ~= "|" then
      cmd = cmd .. "|"
   end

   moves = {}
   for m in cmd:gmatch("%a+[,|]") do 
      moves[#moves+1] = m:sub(1,-2)
      if(m:sub(-1) == '|') then
         r = p:place_road(start, unpack(moves))
         start = r.end_flag
         if create_carriers then
            r:warp_workers{"carrier"}
         end
         moves = {}
      end
   end
end

function place_buildings_with_workers(p, ...)
   for idx,bdescr in ipairs({...}) do
      print(idx,bdescr)
      b = p:place_building(bdescr[1], wl.map.Field(bdescr[2],bdescr[3]))
      print(#b.valid_workers)
      print(b.valid_workers[1])
      b:warp_workers(b.valid_workers)
   end
end
connected_road(p, hq_pos.brn.immovable, "r,r|br,r|r,r")
connected_road(p, hq_pos.brn.immovable, "l,l|l,bl,bl|br,r|br,r|r,tr|tr,tr,tr")

-- Place some buildings from the last map
place_buildings_with_workers(p,
   {"lumberjacks_hut", 15, 11},
   {"lumberjacks_hut", 12, 13},
   {"quarry", 8, 12},
   {"rangers_hut", 9, 13}
) 

use("map", "mission_thread")

