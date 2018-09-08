-- =================================
-- Smugglers Fun Map
-- =================================
include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/objective_utils.lua"
include "scripting/richtext_scenarios.lua"
include "scripting/set.lua"

-- ==========
-- Constants
-- ==========
set_textdomain("mp_scenario_smugglers.wmf")

game = wl.Game()
map = game.map

points_to_win = 2000

-- =================
-- Global Variables
-- =================
points = { 0, 0 }

include "map:scripting/texts.lua"
include "map:scripting/smuggling.lua"

-- ================
-- Initializations
-- ================

-- If the tribes don't have any wares in common, nothing can be smuggled
-- This should not happen, but let's have a safeguard anyway.
function check_ware_compatiblity(player1, player2)
   local has_compatible_ware = false
   for idx,ware in pairs(player1.tribe.wares) do
      if player2.tribe:has_ware(ware.name) then
         has_compatible_ware = true
         break
      end
   end
   if not has_compatible_ware then
      do_game_over()
      return
   end
end

function assign_teams()
   game.players[1].team = 1
   game.players[2].team = 1
   game.players[3].team = 2
   game.players[4].team = 2
   check_ware_compatiblity(game.players[1], game.players[2])
   check_ware_compatiblity(game.players[3], game.players[4])
end

function place_headquarters()
   for idx,player in ipairs(game.players) do
      local sf = map.player_slots[player.number].starting_field

      prefilled_buildings(player, { "barbarians_headquarters", sf.x, sf.y,
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
            pick = 14,
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
            barbarians_carrier = 40,
            barbarians_charcoal_burner = 1,
            barbarians_gardener = 1,
            barbarians_geologist = 4,
            barbarians_lime_burner = 1,
            barbarians_lumberjack = 3,
            barbarians_miner = 4,
            barbarians_ranger = 1,
            barbarians_stonemason = 2,
            barbarians_ox = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 45,
         }
      })
   end
end

function setup_statistics_hook()
   hooks.custom_statistic = {
      name = _"Wares Smuggled",
      pic = "map:genstats_wares_smuggled.png",
      calculator = function(p)
         return points[p.team]
      end,
   }
end

function initialize()
   assign_teams()
   place_headquarters()

   function reveal_smuggling_spots()
      for idx,plr in ipairs(game.players) do
         for idx,region in ipairs(route_descrs) do
            plr:reveal_fields(region.recv)
            plr:reveal_fields(region.send)
            plr:hide_fields(region.recv)
            plr:hide_fields(region.send)
         end
      end
   end

   send_to_all(welcome_msg.body:format((ngettext("%i point", "%i points", points_to_win)):format(points_to_win)), welcome_msg.heading)
   -- set the objective with the game type for all players
   -- TODO change this to a broadcast once individual game objectives have been implementes
   game.players[1]:add_objective("win_conditions", _"Rules", rt(welcome_msg.body:format((ngettext("%i point", "%i points", points_to_win)):format(points_to_win))))

   for idx,descr in ipairs(route_descrs) do
      run(wait_for_established_route, descr)
   end

   wl.Game():add_scenario_hook("smuggling")
   -- Instantiate scenario hook
   hooks.smuggling = {
      hook_function = function()
         print("NOCOM Scenario hook was triggered")
         do_smuggling()
         return
      end,
   }
end

setup_statistics_hook()

run(initialize)
