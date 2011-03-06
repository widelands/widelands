-- =================================
-- Island Hopping Fun Map Scripting 
-- =================================

use("aux", "coroutine")
use("aux", "infrastructure")
use("aux", "formatting")
use("aux", "objective_utils")
use("aux", "set")

-- ==========
-- Constants 
-- ==========
-- TODO: parse the scripts and create a new catalog for it
-- set_textdomain("scenario_atl01.wmf")
game = wl.Game()
map = game.map

use("map", "texts")
use("map", "first_island")

-- ==================
-- Utility functions 
-- ==================
function send_to_all(text)
   for idx,plr in ipairs(game.players) do
      plr:send_message(_ "Game Status", text, {popup=true})
   end
end

function format_rewards(r)
   rv = {}
   for name,count in pairs(r) do
      rv[#rv + 1] = (_"%i %s<br>\n"):format(count, name)
   end
   return table.concat(rv)
end

function add_wares(hq, wares) 
   for name, count in pairs(wares) do
      hq:set_wares(name, hq:get_wares(name) + count)
   end
end

function add_workers(hq, workers) 
   for name, count in pairs(workers) do
      hq:set_workers(name, hq:get_workers(name) + count)
   end
end

-- ===============
-- Initialization 
-- ===============
-- Reveal the whole map for all players
function reveal_everything_for_everybody()
   local fields = {}
   local i = 1
   for x=0, map.width-1 do
      for y=0, map.height-1 do
         fields[i] = map:get_field(x,y)
         i = i + 1
      end
   end

   for idx, plr in ipairs(game.players) do
      plr:reveal_fields(fields)
   end
end

-- Place headquarters for all players
function place_headquarters()
   for idx, plr in ipairs(game.players) do
      local sf = map.player_slots[plr.number].starting_field

      prefilled_buildings(plr, {"headquarters", sf.x, sf.y, 
         wares = {
            diamond = 7,
            ironore = 5,
            quartz = 9,
            stone = 50,
            spideryarn = 9,
            trunk = 20,
            coal = 12,
            gold = 4,
            goldyarn = 6,
            iron = 8,
            planks = 45,
            spidercloth = 5,
            blackroot = 5,
            blackrootflour = 12,
            bread = 8,
            corn = 5,
            cornflour = 12,
            fish = 3,
            meat = 3,
            smoked_fish = 6,
            smoked_meat = 6,
            water = 12,
            bakingtray = 2,
            bucket = 2,
            fire_tongs = 2,
            fishing_net = 4,
            hammer = 11,
            hunting_bow = 1,
            milking_tongs = 2,
            hook_pole = 2,
            pick = 12,
            saw = 9,
            scythe = 4,
            shovel = 9,
            tabard = 5,
            light_trident = 5,
         },
         workers = {
            armoursmith = 1,
            blackroot_farmer = 1,
            builder = 10,
            burner = 1,
            carrier = 40,
            fish_breeder = 1,
            geologist = 4,
            miner = 4,
            sawyer = 1,
            stonecutter = 2,
            toolsmith = 2,
            weaponsmith = 1,
            woodcutter = 3,
            horse = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 35,
         }
      })
   end
end

-- Game initializations
function initialize()
   reveal_everything_for_everybody()
   place_headquarters()

   for idx,plr in ipairs(game.players) do
      run(function() run_island1(plr) end)
   end
end


run(initialize)
