-- =======================================================================
--                            Tutorial Mission
-- =======================================================================

-- TODO: move common functions to tutorial_utils.lua or so
-- compare with other stuff (documentation, redundant functions etc.)


-- ===============
-- Initialization
-- ===============

plr = wl.Game().players[1]
plr2 = wl.Game().players[2]
plr:forbid_buildings{"scouts_house"} -- otherwise, player could scout and attack the enemy when he should not
map = wl.Game().map
mv = wl.ui.MapView()

tavern_field = map:get_field(105,44)
warehouse_field = map:get_field(78,67)

sf = map.player_slots[1].starting_field

set_textdomain("economy.wmf")

include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/ui.lua"
include "scripting/table.lua"

include "map:scripting/texts.lua"


function init_player()
   -- a normal headquarters, but without helms, lances, ores, coal
   prefilled_buildings(plr, { "headquarters", sf.x, sf.y,
      wares = {
         ax = 6,
         bread_paddle = 2,
         basket = 2,
         bread = 28, -- was 8
         cloth = 5,
         fire_tongs = 2,
         fish = 6,
         fishing_rod = 2,
         flour = 34, -- was 4
         grape = 4,
         hammer = 14,
         hunting_spear = 2,
         kitchen_tools = 4,
         marble = 25,
         marblecolumn = 6,
         meal = 4,
         meat = 6,
         pick = 8,
         ration = 5, -- was 12
         saw = 2,
         scythe = 5,
         shovel = 6,
         stone = 40,
         log = 60, --was 30
         water = 12,
         wheat = 24, -- was 4
         wine = 8,
         wood = 45,
         wool = 2,
      },
      workers = {
         armorsmith = 1,
         brewer = 1,
         builder = 10,
         carrier = 40,
         charcoal_burner = 1,
         geologist = 4,
         lumberjack = 3,
         miner = 4,
         stonemason = 2,
         toolsmith = 2,
         weaponsmith = 1,
         donkey = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 0, -- was 45
      }
   })
   
   filled_buildings_one_soldier(plr,
      {"lumberjacks_house",92,65},
      {"lumberjacks_house",89,52},
      {"foresters_house",90,54},
      {"foresters_house",91,56},
      {"lumberjacks_house",92,57},
      {"foresters_house",93,59},
      {"lumberjacks_house",93,61},
      {"quarry",99,57},
      {"quarry",89,38},
      {"sawmill",91,53},
      {"stonemasons_house",92,50},
      {"donkeyfarm",96,58},
      {"farm",88,48},
      {"farm",91,38},
      {"farm",94,39},
      {"farm",88,34},
      {"vineyard",97,49},
      {"vineyard",100,50},
      {"vineyard",103,50},
      {"vineyard",103,56},
      {"winery",101,52},
      {"winery",101,57},
      {"brewery",106,49},
      {"brewery",104,49},
      {"piggery",110,23},
      {"piggery",113,21},
      {"warehouse",110,29,
         wares = {
            wheat = 20,
            flour = 20
         }
      },
      {"mill",105,46},
      {"mill",107,44},
      {"mill",108,39},
      {"bakery",114,26},
      {"bakery",116,28},
      {"bakery",115,32},
      {"tavern",tavern_field.x,tavern_field.y}, -- (105,44), will be destroyed
      {"coalmine",118,45, wares = {beer = 6}},
      {"coalmine",119,39, wares = {beer = 6}},
      {"oremine",107,59, wares = {beer = 6}},
      {"marblemine",98,38, wares = {wine = 6}},
      {"marblemine",102,38, wares = {wine = 6}},
      {"smelting_works",110,38, wares = {}},
      {"smelting_works",111,43, wares = {}},
      {"toolsmithy",104,64, wares = {log = 8}},
      {"weaponsmithy",113,40, wares = {wood = 8}},
      {"armorsmithy",112,37, wares = {cloth = 8}},
      {"farm",105,70},
      {"farm",101,71},
      {"farm",99,77},
      {"fishers_house",106,77},
      {"fishers_house",104,77},
      {"fishers_house",103,79},
      {"warehouse",100,74},
      {"hunters_house",95,60},
      {"well",92,48},
      {"well",103,45},
      {"well",107,40},
      {"well",103,47},
      {"well",104,51},
      {"well",106,51},
      {"well",113,23},
      {"well",113,32},
      {"well",112,34},
      {"well",111,31},
      {"well",115,24},
      {"warehouse",warehouse_field.x,warehouse_field.y}, -- (78,67)
      
      {"barracks",89,31},
      {"barrier",91,45},
      {"tower",103,42},
      {"barrier",112,25},
      {"sentry",116,41},
      {"barracks",102,66},
      {"outpost",103,76},
      {"tower",90,67},
      {"sentry",105,56},
      {"fortress",66,63},
      {"fortress",71,66},
      {"fortress",75,72}
   )
   plr:conquer(map:get_field(111,34),3)
   
   connected_road(plr,map:get_field(97,54).immovable,"tr,tr|tr,tl")
   connected_road(plr,map:get_field(98,52).immovable,"br,r|r,r|tr,r,tr|tr,r|r,r|tr,tr|tr,tr|tr,tr|r,r|r,r|r,r|br,br|r,r")
   connected_road(plr,map:get_field(99,53).immovable,"tr,tr|tr,tr|tl,tr|tr,tr|tr,tr|tr,tl|tl,tl|l,l|l,l|l,l|l,bl")
   connected_road(plr,map:get_field(97,54).immovable,"bl,br|br,bl,bl|bl,bl|bl,l|bl,bl|br,bl|bl,bl,l|l,l|l,l|l,l|l,l|l,l|l,l|l,l|l,l|l,l|l,tl|l,l|l,tl|tl,tl")
   connected_road(plr,map:get_field(97,56).immovable,"r,br|br,r|r,r|r,tr|r,r|br,br|br,r")
   connected_road(plr,map:get_field(97,54).immovable,"l,l|l,tl|tl,tl|bl,l|l,bl")
   connected_road(plr,map:get_field(88,49).immovable,"br,br|r,br")
   connected_road(plr,map:get_field(92,51).immovable,"tr,tl|tr,tl,tl|tr,tr|tr,tr|tr,tr")
   connected_road(plr,map:get_field(94,42).immovable,"tl,l|tl,tl|l,l|l,tl,tl|tr,tr|tr,tr,tr")
   connected_road(plr,map:get_field(75,73).immovable,"tr,tr|tr,r|tr,tr")
   connected_road(plr,map:get_field(92,54).immovable,"tr,r")
   connected_road(plr,map:get_field(93,58).immovable,"tr,tr|tr,tr")
   connected_road(plr,map:get_field(102,45).immovable,"br,r|br,r|br,bl,bl")
   connected_road(plr,map:get_field(105,47).immovable,"tr,tl|r,r|br,r")
   connected_road(plr,map:get_field(116,44).immovable,"tr,tr|tr,tr|r,r")
   connected_road(plr,map:get_field(105,45).immovable,"tr,tr|tr,tr|r,tr|tr,r|tr,tr|tr,tr|tr,tr|r,r|tr,tr|tr,tl|l,tl,tl|l,tl|l,tl,tl")
   connected_road(plr,map:get_field(114,22).immovable,"br,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,bl|bl,l")
   connected_road(plr,map:get_field(105,42).immovable,"br,r")
   connected_road(plr,map:get_field(111,37).immovable,"br,r|r,br|bl,bl|bl,bl,bl")
   connected_road(plr,map:get_field(106,59).immovable,"bl,bl|bl,br|bl,bl|l,bl,bl|bl,br|br,br|r,r|br,bl|br,br|br,br,bl|l,l|bl,bl|l,tl|tl,tl|tl,tl|tr,tr,tr|tr,r")
   connected_road(plr,map:get_field(100,78).immovable,"tr,r|r,r")
   connected_road(plr,map:get_field(104,48).immovable,"r,tr")
   connected_road(plr,map:get_field(105,52).immovable,"tr,tl")
   connected_road(plr,map:get_field(107,52).immovable,"tr,tl")
   connected_road(plr,map:get_field(112,32).immovable,"l,l")
   connected_road(plr,map:get_field(115,25).immovable,"bl,bl")
   connected_road(plr,map:get_field(97,54).immovable,"r,r,tr")
   
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

   if i.position == "topleft" then
      i.posx = 0
      i.posy = 0
   elseif i.position == "topright" then
      i.posx = 10000
      i.posy = 0
   end

   plr:message_box(i.title, i.body, i)

   local o = _try_add_objective(i)

   sleep(130)

   return o
end

-- A copy of prefilled_buildings from scripting/infrastructure.lua, but with
-- as much wares as possible, if not indicated otherwise, and only one soldier
function filled_buildings_one_soldier(p, ...)
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
            -- TODO(codereview): this creates "1 soldier (+4)", but I want a capacity of 1
            b:set_soldiers({0,0,0,0}, 1)
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
   sleep(500)
   msg_box(intro1)
   msg_box(intro2)
   
   burn_tavern_down()
end

function burn_tavern_down()
   sleep(500)
   scroll_smoothly_to(tavern_field)
   sleep(250)
   tavern_field.immovable:destroy()
   sleep(50)
   msg_box(tavern_burnt_down)
   local o = msg_box(building_stat)
   -- wait for the window to open and close
   while not mv.windows.building_statistics do sleep(100) end
   while mv.windows.building_statistics do sleep(100) end
   o.done = true
   
   o = msg_box(inventory1)
   while not mv.windows.stock_menu do sleep(200) end
   o.done = true
   msg_box(inventory2)
   
   o = msg_box(build_taverns)
   
   sleep(100*1000)
   msg_box(ware_encyclopedia) -- a small insert
   
   while #plr:get_buildings("tavern") < 2 do sleep(500) end
   o.done = true
   
   plan_the_future()
end

function plan_the_future()
   sleep(200)
   msg_box(building_priority_settings)
   msg_box(ware_stat)
   --while not mv.windows.ware_statistics do sleep(200) end
   -- TODO(codereview): How do I select a ware via scripting?
   -- TODO(wl-zocker): If possible, choose an example (wheat?) and talk about it. Make objective for opening
   local o = msg_box(economy_settings1)
   while not mv.windows.economy_options do sleep(200) end
   o.done = true
   o = msg_box(economy_settings2)
   
   sleep(40*1000)
   
   o.visible = false
   -- just forget about the old objective, the new one includes the old one
   o = msg_box(warehouse_preference_settings)
   
   local enough_wares = false
   while not enough_wares do
      if (warehouse_field.immovable and (warehouse_field.immovable.descr.name == "warehouse")) then
         if warehouse_field.immovable:get_wares("marblecolumn") >= 20 then
            enough_wares = true
         end
      end
      sleep(500)
   end
   o.done = true
   
   -- if the minimum_storage_per_warehouse feature is introduced, use the gold mountain to the northeast for explanation
   conclude()
end

function conclude()
   msg_box(conclusion)
end

function test()
   local n = warehouse_field.immovable:get_wares('log')
   print(n)
end

run(init_player)
run(introduction)
--run(plan_the_future)
--run(test)