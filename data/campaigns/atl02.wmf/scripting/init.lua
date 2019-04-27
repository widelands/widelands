-- =======================================================================
--                      Atlanteans Tutorial Mission 02
-- =======================================================================
include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/table.lua"
include "scripting/ui.lua"

include "map:scripting/infrastructure_plr3.lua"

-- ===================
-- Constants & Config
-- ===================
set_textdomain("scenario_atl02.wmf")

game = wl.Game()
map = game.map
p1 = game.players[1]
kalitath = game.players[3]
maletus = game.players[2]

-- =================
-- global variables
-- =================

include "map:scripting/texts.lua"

-- =================
-- Helper functions
-- =================
-- Show one message box
function msg_box(i)
   if i.pre_func then i.pre_func() end

   if not i.h then i.h = 400 end

   p1:message_box(i.title, i.body, i)

   if i.post_func then i.post_func() end

   sleep(130)
end

-- Show many message boxes
function msg_boxes(boxes_descr)
   for idx,box_descr in ipairs(boxes_descr) do
      msg_box(box_descr)
   end
end

-- Add an objective
function add_obj(o)
   return p1:add_objective(o.name, o.title, o.body)
end

-- ===============
-- Initialization
-- ===============
function _initialize_player(args)
   p1:allow_buildings("all")
   p1:allow_workers("all")

   p1:forbid_buildings{"atlanteans_shipyard"}

   local sf = map.player_slots[1].starting_field
   prefilled_buildings(p1, { "atlanteans_headquarters", sf.x, sf.y,
      wares = {
         diamond = 2,
         iron_ore = 50,
         quartz = 3,
         granite = 8,
         spider_silk = 4,
         log = 40,
         coal = 150,
         gold = 0,
         gold_thread = 0,
         iron = 0,
         planks = 23,
         spidercloth = 5,
         blackroot = 0,
         blackroot_flour = 20,
         atlanteans_bread = 10,
         corn = 0,
         cornmeal = 20,
         fish = 0,
         meat = 0,
         smoked_fish = 16,
         smoked_meat = 17,
         water = 4,
         bread_paddle = 0,
         buckets = 0,
         fire_tongs = 0,
         fishing_net = 0,
         hammer = 0,
         hunting_bow = 0,
         milking_tongs = 0,
         hook_pole = 0,
         pick = 0,
         saw = 0,
         scythe = 0,
         shovel = 0,
         tabard = 0,
         trident_light = 0,
      },
      workers = {
         atlanteans_armorsmith = 0,
         atlanteans_blackroot_farmer = 0,
         atlanteans_builder = 10,
         atlanteans_charcoal_burner = 0,
         atlanteans_carrier = 40,
         atlanteans_fishbreeder = 1,
         atlanteans_geologist = 4,
         atlanteans_miner = 2,
         atlanteans_sawyer = 1,
         atlanteans_smelter = 1,
         atlanteans_stonecutter = 1,
         atlanteans_toolsmith = 1,
         atlanteans_weaponsmith = 0,
         atlanteans_woodcutter = 3,
         atlanteans_horse = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 10,
      }
   })
end
function _initialize_kalitath()
   kalitath:allow_buildings("all")

   -- A default headquarters
   local sf = map.player_slots[3].starting_field
   hq = prefilled_buildings(kalitath, { "barbarians_headquarters", sf.x, sf.y,
      wares = {
         ax = 5,
         bread_paddle = 2,
         blackwood = 32,
         cloth = 5,
         coal = 12,
         felling_ax = 4,
         fire_tongs = 2,
         fish = 6,
         fishing_rod = 2,
         gold = 4,
         grout = 12,
         hammer = 12,
         hunting_spear = 2,
         iron = 12,
         iron_ore = 5,
         kitchen_tools = 4,
         meal = 4,
         meat = 6,
         pick = 8,
         barbarians_bread = 8,
         ration = 12,
         granite = 40,
         scythe = 6,
         shovel = 4,
         snack = 3,
         thatch_reed = 24,
         log = 80,
      },
      workers = {
         barbarians_blacksmith = 2,
         barbarians_brewer = 1,
         barbarians_builder = 10,
         barbarians_charcoal_burner = 1,
         barbarians_carrier = 40,
         barbarians_gardener = 1,
         barbarians_geologist = 4,
         barbarians_lime_burner = 1,
         barbarians_lumberjack = 3,
         barbarians_miner = 4,
         barbarians_miner_master = 4,
         barbarians_ranger = 1,
         barbarians_stonemason = 2,
         barbarians_trainer = 3,
         barbarians_ox = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 45,
      }
   }) 
   set_textdomain("scenario_atl02.wmf")
end
function _initialize_maletus(args)
   maletus:allow_buildings("all")

   local sf = map.player_slots[2].starting_field
   -- A default headquarters
   prefilled_buildings(maletus, { "empire_headquarters", sf.x, sf.y,
      wares = {
         armor_helmet = 4,
         spear_wooden = 5,
         felling_ax = 6,
         bread_paddle = 2,
         basket = 2,
         empire_bread = 8,
         cloth = 5,
         coal = 12,
         fire_tongs = 2,
         fish = 6,
         fishing_rod = 2,
         flour = 4,
         gold = 4,
         grape = 4,
         hammer = 14,
         hunting_spear = 2,
         iron = 12,
         iron_ore = 5,
         kitchen_tools = 4,
         marble = 25,
         marble_column = 6,
         meal = 4,
         meat = 6,
         pick = 8,
         ration = 12,
         saw = 2,
         scythe = 5,
         shovel = 6,
         granite = 40,
         log = 30,
         water = 12,
         wheat = 4,
         wine = 8,
         planks = 45,
         wool = 2,
      },
      workers = {
         empire_armorsmith = 1,
         empire_brewer = 1,
         empire_builder = 10,
         empire_carrier = 40,
         empire_charcoal_burner = 1,
         empire_geologist = 4,
         empire_lumberjack = 3,
         empire_miner = 4,
         empire_stonemason = 2,
         empire_toolsmith = 2,
         empire_trainer = 3,
         empire_weaponsmith = 1,
         empire_donkey = 5,
      },
      soldiers = {
         [{0,0,0,0}] = 45,
      }
   })
   set_textdomain("scenario_atl02.wmf")
end

function initialize()
   _initialize_player()
   _initialize_maletus()
   _initialize_kalitath()
end

-- ==============
-- Logic Threads
-- ==============
function intro()
   sleep(1000)

   p1.see_all = true -- TODO: remove this
   initialize()

   add_obj(obj_basic_infrastructure)
   add_obj(obj_tools)
   add_obj(obj_explore)
   -- msg_boxes(initial_messages)

   create_economy_for_plr_3()
end

run(intro)

