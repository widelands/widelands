-- =================================
-- Smugglers Fun Map
-- =================================
include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/formatting.lua"
include "scripting/objective_utils.lua"
include "scripting/set.lua"

-- ==========
-- Constants
-- ==========
set_textdomain("mp_scenario_smugglers.wmf")

game = wl.Game()
map = game.map

points_to_win = 2000

route_descrs = {
   { value = 2, send = map:get_field(35, 52):region(2), recv = map:get_field(96, 77):region(2) },
   { value = 2, send = map:get_field(98, 55):region(2), recv = map:get_field(34, 76):region(2) },

   { value = 3, send = map:get_field(64, 57):region(2), recv = map:get_field(78, 73):region(2) },
   { value = 3, send = map:get_field(77, 58):region(2), recv = map:get_field(65, 72):region(2) },

   { value = 2, send = map:get_field(62, 93):region(2), recv = map:get_field(78, 34):region(2) },
   { value = 2, send = map:get_field(80, 95):region(2), recv = map:get_field(63, 29):region(2) },
   { value = 2, send = map:get_field(18, 66):region(2), recv = map:get_field(121, 61):region(2) },
   { value = 2, send = map:get_field(124, 72):region(2), recv = map:get_field(19, 57):region(2) }
}

-- =================
-- Global Variables
-- =================
points = { 0, 0 }

-- =================
-- Utility functions
-- =================
function send_to_all(text)
   for idx,plr in ipairs(game.players) do
      plr:send_message(_ "Game Status", text, {popup=true})
   end
end

include "map:scripting/texts.lua"
include "map:scripting/smuggling.lua"

-- ================
-- Initializations
-- ================
function assign_teams()
   game.players[1].team = 1
   game.players[2].team = 1
   game.players[3].team = 2
   game.players[4].team = 2
end

function place_headquarters()
   for idx,player in ipairs(game.players) do
      local sf = map.player_slots[player.number].starting_field

      prefilled_buildings(player, { "headquarters", sf.x, sf.y,
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
            ironore = 5,
            kitchen_tools = 4,
            meal = 4,
            meat = 6,
            pick = 14,
            pittabread = 8,
            ration = 12,
            raw_stone = 40,
            scythe = 6,
            shovel = 4,
            snack = 3,
            thatchreed = 24,
            log = 80,
         },
         workers = {
            blacksmith = 2,
            brewer = 1,
            builder = 10,
            burner = 1,
            carrier = 40,
            gardener = 1,
            geologist = 4,
            ["lime-burner"] = 1,
            lumberjack = 3,
            miner = 4,
            ranger = 1,
            stonemason = 2,
            ox = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 45,
         }
      })
   end
end

function setup_statistics_hook()
	if hooks == nil then hooks = {} end
	hooks.custom_statistic = {
      name = _ "Wares Smuggled",
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

   send_to_all(welcome_msg:format((ngettext("%i point", "%i points", points_to_win)):format(points_to_win)))


   for idx,descr in ipairs(route_descrs) do
      run(wait_for_established_route, descr)
   end
end

setup_statistics_hook()

run(initialize)
