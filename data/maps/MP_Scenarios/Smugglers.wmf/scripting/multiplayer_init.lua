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
push_textdomain("mp_scenario_smugglers.wmf")

game = wl.Game()
map = game.map
game.allow_diplomacy = false

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
   include "map:scripting/starting_conditions.lua"
   for idx,player in ipairs(game.players) do
      local sf = map.player_slots[player.number].starting_field
      local tribename = player.tribe.name
      if tribename == "barbarians" then
         set_starting_conditions_barbarians(player, sf)
      elseif tribename == "empire" then
         set_starting_conditions_empire(player, sf)
      elseif tribename == "atlanteans" then
         set_starting_conditions_atlanteans(player, sf)
      elseif tribename == "frisians" then
         set_starting_conditions_frisians(player, sf)
      else
         print("We don't have starting conditions for tribe " .. tribename)
         do_game_over()
      end
   end
end

function setup_statistics_hook()
   hooks.custom_statistic = {
      name = _("Wares Smuggled"),
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

   send_to_all_inboxes(welcome_msg.body:format((ngettext("%i point", "%i points", points_to_win)):format(points_to_win)), welcome_msg.heading)
   -- set the objective with the game type for all players
   -- TODO change this to a broadcast once individual game objectives have been implementes
   game.players[1]:add_objective("win_conditions", _("Rules"), rt(welcome_msg.body:format((ngettext("%i point", "%i points", points_to_win)):format(points_to_win))))

   for idx,descr in ipairs(route_descrs) do
      run(wait_for_established_route, descr)
   end

   do_smuggling()
end

setup_statistics_hook()

run(initialize)
