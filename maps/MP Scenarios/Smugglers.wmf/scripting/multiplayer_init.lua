-- =================================
-- Smugglers Fun Map
-- =================================
use("aux", "coroutine")
use("aux", "infrastructure")
use("aux", "formatting")
use("aux", "objective_utils")
use("aux", "set")

-- ==========
-- Constants 
-- ==========
set_textdomain("mp_scenario_smugglers.wmf")

game = wl.Game()
map = game.map

wares_to_smuggle = 150 -- How many are needed to win?

warehouse_regions = {
   map:get_field(52, 50):region(2), -- Blue player,
   map:get_field(59, 50):region(2), -- Red player,
   map:get_field(59, 60):region(2), -- Yellow player,
   map:get_field(52, 60):region(2), -- Green player,
}

-- =================
-- Global Variables 
-- =================
wares_smuggled = { 0, 0 }

-- =================
-- Utility functions
-- =================
function send_to_all(text)
   for idx,plr in ipairs(game.players) do
      plr:send_message(_ "Game Status", text, {popup=true})
   end
end

use("map", "texts")
use("map", "smuggling")

-- ================
-- Initializations 
-- ================
function assign_teams()
   game.players[1].team = 1
   game.players[3].team = 1
   game.players[2].team = 2
   game.players[4].team = 2
end

function place_headquarters()
   for idx,player in ipairs(game.players) do
      local sf = map.player_slots[player.number].starting_field

      prefilled_buildings(player, { "headquarters", sf.x, sf.y,
         wares = {
            axe = 5,
            bakingtray = 2,
            blackwood = 32,
            cloth = 5,
            coal = 12,
            felling_axe = 4,
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
            trunk = 80,
         },
         workers = {
            blacksmith = 2,
            brewer = 1,
            builder = 10,
            burner = 1,
            carrier = 40,
            ferner = 1,
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

function show_middle_to_everybody()
   local area = map:get_field(55,55):region(12)

   for idx,plr in ipairs(game.players) do
      plr:reveal_fields(area)
      plr:hide_fields(area)
   end
end

function setup_statistics_hook()
	if hooks == nil then hooks = {} end
	hooks.custom_statistic = {
      name = _ "Wares smuggled",
      pic = "map:genstats_wares_smuggled.png",
      calculator = function(p) 
         return wares_smuggled[p.team]
      end,
   }
end

function initialize()
   assign_teams()
   place_headquarters()
   show_middle_to_everybody()

   send_to_all(welcome_msg:format(wares_to_smuggle))

   run(wait_for_established_route, game.players[1], game.players[3])
   run(wait_for_established_route, game.players[2], game.players[4])
end

setup_statistics_hook()

run(initialize)
