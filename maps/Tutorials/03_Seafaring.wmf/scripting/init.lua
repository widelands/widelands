-- =======================================================================
--                            Tutorial Mission
-- =======================================================================

-- TODO: move common functions to tutorial_utils.lua or so
-- compare with other stuff (documentation, redundant functions etc.)


-- ===============
-- Initialization
-- ===============

plr = wl.Game().players[1]
plr:allow_buildings("all")
map = wl.Game().map

sf = map.player_slots[1].starting_field
second_port_field = map:get_field(37,27)
port_on_island = map:get_field(102,36)
additional_port_space = map:get_field(85,5)


set_textdomain("seafaring.wmf")

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/ui.lua"
include "scripting/table.lua"

include "map:scripting/texts.lua"


function init_player()
   filled_buildings(plr, {"port", sf.x, sf.y,
      wares = {
         diamond = 7,
         ironore = 20,
         quartz = 9,
         stone = 50,
         spideryarn = 9,
         log = 80,
         coal = 12,
         goldyarn = 6,
         iron = 12,
         planks = 65,
         spidercloth = 35,
         blackroot = 5,
         blackrootflour = 12,
         bread = 28,
         corn = 15,
         cornflour = 12,
         fish = 13,
         meat = 13,
         smoked_fish = 26,
         smoked_meat = 26,
         water = 12,
         bread_paddle = 2,
         bucket = 2,
         fire_tongs = 2,
         fishing_net = 4,
         hammer = 11,
         hunting_bow = 1,
         milking_tongs = 2,
         hook_pole = 2,
         pick = 8,
         saw = 9,
         scythe = 4,
         shovel = 9,
         tabard = 15,
         light_trident = 15,
      },
      workers = {
         armorsmith = 1,
         blackroot_farmer = 1,
         builder = 10,
         charcoal_burner = 1,
         carrier = 40,
         fish_breeder = 2,
         geologist = 4,
         miner = 3,
         sawyer = 1,
         stonecutter = 2,
         toolsmith = 2,
         weaponsmith = 1,
         woodcutter = 3,
         horse = 15,
      },
      soldiers = {
         [{0,0,0,0}] = 35,
      }
   },
   {"woodcutters_house", 32, 48},
   {"woodcutters_house", 33, 47},
   {"woodcutters_house", 29, 56},
   {"woodcutters_house", 30, 57},
   {"foresters_house", 31, 53},
   {"foresters_house", 32, 46},
   {"foresters_house", 32, 50},
   {"foresters_house", 34, 50},
   {"sawmill", 41, 46},
   {"quarry",  45, 54},
   {"tower", 43, 52},
   {"tower", 35, 46},
   {"guardhouse", 49, 60},
   {"crystalmine", 51, 61},
   {"guardhall", 43, 64},
   {"fish_breeders_house", 40, 66},
   {"fishers_house", 42, 66},
   {"fish_breeders_house", 44, 66},
   {"fishers_house", 46, 66},
   {"fish_breeders_house", 48, 66},   
   {"castle", 22, 55},
   {"coalmine", 22, 48},
   {"coalmine", 16, 49},
   {"farm", 20, 59},
   {"farm", 23, 59},
   {"farm", 19, 64},
   {"farm", 21, 65},
   {"farm", 26, 64},
   {"blackroot_farm", 53, 53},
   {"blackroot_farm", 56, 50},
   {"mill", 34, 58},
   {"mill", 32, 64},
   {"bakery", 38, 49},
   {"bakery", 40, 48},
   {"smokery", 46, 57},
   {"smokery", 44, 57},
   {"horsefarm", 40, 55},
   {"spiderfarm", 37, 45},
   {"weaving-mill", 45, 45},
   {"smelting_works", 35, 56, wares = {coal = 8, ironore = 8}}, -- no gold
   {"smelting_works", 35, 59, wares = {coal = 8, ironore = 8}},
   {"toolsmithy", 41, 52},
   {"weaponsmithy", 37, 54},
   {"small_tower", 34, 63},
   {"tower", 52, 46},
   {"well", 39, 43},
   {"well", 39, 45},
   {"well", 38, 55},
   {"well", 35, 61},
   {"well", 32, 59},
   {"hunters_house", 37, 50},
   {"warehouse", 36, 57,
      wares = {},
      workers = {},
      soldiers = {}
   },
   
   {"castle", 36, 20},
   {"warehouse", 37, 16, 
      wares = {
         smoked_meat = 50,
         smoked_fish = 150,
         bread = 200,
         log = 20,
         planks = 13,
         stone = 10,
         diamond = 3,
         quartz = 4,
         spidercloth = 7,
         gold = 2,
      },
      workers = {
         builder = 3,
         miner = 3
      },
      soldiers = {
         [{0,0,0,0}] = 10,
      }
   }, -- end of warehouse on northern peninsula
   {"goldmine", 32, 25}   
   )
   
   plr:forbid_buildings{"shipyard"}
   
   connected_road(plr,map:get_field(42,44).immovable,"bl,br,bl|bl,bl|bl,l|l,bl|l,l,l|l,l|tr,tl|r,tr")
   connected_road(plr,map:get_field(42,44).immovable,"br,r|br,r,r|r,r,tr|r,r|r,r|br,bl|br,br|br,br|r,r")
   connected_road(plr,map:get_field(35,47).immovable,"r,r,tr|r,r|br,r|br,br|br,br|br,bl|br,br,r|br,r|br,bl|br,br|br,r|r,br,r")
   connected_road(plr,map:get_field(45,58).immovable,"r,r|bl,br,bl|bl,bl,l|bl,bl|bl,bl")
   connected_road(plr,map:get_field(48,67).immovable,"l,l|l,l|l,l|l,l|l,tl|tl,tl|l,l,l|l,l,bl|l,tl,l|l,bl|l,l|bl,l,l|l,l|l,l,tl|tr,tr|tl,tr,tr|tr,tr|tr,tr|tr,tr|tr,tr|tr,tr|tr,tr|tr,tr|tr,tr|tr,r|r,r|r,r|br,br|br,r,r")
   connected_road(plr,map:get_field(43,53).immovable,"l,l|br,bl,bl|l,l|l,tl|bl,l,bl|bl,bl|br,r|br,bl|bl,bl")
   connected_road(plr,map:get_field(17,50).immovable,"r,r|r,r|tr,r|br,r|br,br|r,br|br,br|br,br,r|br,r|bl,br|r,r|tr,r")
   connected_road(plr,map:get_field(24,60).immovable,"l,l,l")
   connected_road(plr,map:get_field(54,54).immovable,"tr,tr,tl")
   connected_road(plr,map:get_field(40,44).immovable,"r,r")
   connected_road(plr,map:get_field(37,58).immovable,"bl,bl")

   connected_road(plr,map:get_field(33,26).immovable,"r,tr|tr,tr|tr,tr|tr,tr|tl,tr")
   
end


-- =================
-- Helper functions
-- =================

function _try_add_objective(i)
   -- Add an objective that is defined in the table i to the players objectives.
   -- Returns the new objective or nil. Does nothing if i does not specify an
   -- objective.
   local o = nil
   if i.obj_name then
      o = plr:add_objective(i.obj_name, i.obj_title, i.obj_body)
   end
   return o
end

function msg_box(i)
   if i.field then
      scroll_smoothly_to(i.field.trn.trn.trn.trn)
      -- TODO: improve this, depending on topleft/topright, scroll only left/right

      i.field = nil -- Otherwise message box jumps back
   end

   if i.pos == "topleft" then
      i.posx = 0
      i.posy = 0
   elseif i.pos == "topright" then
      i.posx = 10000
      i.posy = 0
   end

   plr:message_box(i.title, i.body, i)

   local o = _try_add_objective(i)

   sleep(130)

   return o
end

-- A copy of prefilled_buildings from scripting/infrastructure.lua, but with
-- as much wares as possible, if not indicated otherwise
function filled_buildings(p, ...)
   for idx,bdescr in ipairs({...}) do
      b = p:place_building(bdescr[1], wl.Game().map:get_field(bdescr[2],bdescr[3]), false, true)
      -- Fill with workers
      if b.valid_workers then b:set_workers(b.valid_workers) end
      if bdescr.workers then b:set_workers(bdescr.workers) end
      -- Fill with soldiers
      if b.max_soldiers and b.set_soldiers then
         if bdescr.soldiers then
            b:set_soldiers(bdescr.soldiers)
         else
            b:set_soldiers({0,0,0,0}, b.max_soldiers)
         end
      elseif bdescr.soldiers then -- Must be a warehouse
         b:set_soldiers(bdescr.soldiers)
      end
      -- Fill with wares
      if bdescr.wares then b:set_wares(bdescr.wares)
      elseif b.valid_wares then b:set_wares(b.valid_wares) end
   end
end

-- =============
-- Actual script
-- =============

function introduction()
   additional_port_space.terr = "wasser" -- disable the port space
   sleep(1000)
   msg_box(intro_south)
   sleep(300)
   msg_box(intro_north)
   sleep(20000)
   
   build_port()
end

function build_port()
   sleep(200)
   msg_box(tell_about_port)
   
   wl.ui.MapView().buildhelp = true -- so that the player sees the port space
   local o = msg_box(tell_about_port_building)
   
   while #plr:get_buildings("port") < 2 do sleep(200) end
   o.done = true
   
   build_ships()
end

function build_ships()
   sleep(200)
   local o = msg_box(tell_about_shipyard)
   plr:allow_buildings{"shipyard"}
   
   while #plr:get_buildings("shipyard") < 1 do sleep(200) end
   o.done = true
   
   local o = msg_box(tell_about_ships)
   
   -- TODO(codereview): how to check for ships? (player shall build two)
   sleep(30*60*1000) -- 30 minutes
   o.done = true
   
   expedition()
end

function expedition()
   sleep(200)
   msg_box(expedition1)
   local o = msg_box(expedition2)
   
   -- TODO(codereview): Can I check whether a ship is ready for an expedition?
   sleep(2*60*1000) -- 2 minutes
   o.done = true
   
   local o2 = msg_box(expedition3)
   
   while #plr:get_buildings("port") < 3 do sleep(200) end
   o.done = true
   
   -- places 5 signs with iron to show the player he really found some iron ore
   local fields = map:get_field(97,35):region(3)
   for i=1,5 do
      local successful = false
      while not successful do
         local idx = math.random(#fields)
         f = fields[idx]
         if ((f.resource == "iron") and not f.immovable) then
            map:place_immovable("resi_iron2",f,"atlanteans")
            successful = true
         end
         table.remove(fields,idx)
      end
   end
   
   conclude()
end

function conclude()
   additional_port_space.terr = "desert_steppe" -- make it land again so that the player can build a port
   msg_box(conclusion)
end   

run(init_player)
run(introduction)