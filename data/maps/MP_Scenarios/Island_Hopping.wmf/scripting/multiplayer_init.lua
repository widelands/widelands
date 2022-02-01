-- =================================
-- Island Hopping Fun Map Scripting
-- =================================
include "scripting/coroutine.lua"
include "scripting/infrastructure.lua"
include "scripting/messages.lua"
include "scripting/objective_utils.lua"
include "scripting/richtext_scenarios.lua"

-- ==========
-- Constants
-- ==========
push_textdomain("mp_scenario_island_hopping.wmf")

game = wl.Game()
map = game.map

_nplayers_finished_island = {0, 0}
_start_fields = {
   { -- Island 1
      map.player_slots[1].starting_field,
      map.player_slots[2].starting_field,
      map.player_slots[3].starting_field,
      map.player_slots[4].starting_field
   },
   { -- Island 2
      map:get_field(143, 148),
      map:get_field(142,  45),
      map:get_field( 51,  44),
      map:get_field( 49, 147)
   },
   { -- Island 3
      map:get_field(180, 182),
      map:get_field(180,  10),
      map:get_field( 13,   9),
      map:get_field( 13, 182)
   }
}

_finish_areas = {
   { -- Island 1
      map:get_field(136,125):region(3), -- player 1
      map:get_field(136, 70):region(3), -- player 2
      map:get_field( 57, 68):region(3), -- player 3
      map:get_field( 56,122):region(3)  -- player 4
   },
   { -- Island 2
      map:get_field(167,164):region(3), -- player 1
      map:get_field(167, 28):region(3), -- player 2
      map:get_field( 27, 27):region(3), -- player 3
      map:get_field( 26,161):region(3)  -- player 4
   }
}

_finish_rewards = {
   { -- Island 1
      { -- 1st to finish
         log = 25, planks = 15, granite = 10,
         spidercloth = 5, corn = 20,
      },
      { -- 2st to finish
         log = 45, planks = 30, granite = 20,
         spidercloth = 7, corn = 25,
      },
      { -- 3rd to finish
         log = 65, planks = 45, granite = 30,
         spidercloth = 9, corn = 30,
      },
      { -- 4th to finish
         log = 85, planks = 50, granite = 40,
         spidercloth = 11, corn = 35,
      }
   },
   { -- Island 2
      { -- 1st to finish
         coal = 20, iron_ore = 10, gold_ore = 10,
      },
      { -- 2st to finish
         coal = 30, iron_ore = 15, gold_ore = 15,
      },
      { -- 3rd to finish
         coal = 40, iron_ore = 20, gold_ore = 20,
      },
      { -- 4th to finish
         coal = 50, iron_ore = 25, gold_ore = 25,
      }
   }
}
hill = map:get_field(0,0):region(3)

-- ==================
-- Utility functions
-- ==================

-- Returns a list of rewards from _finish_rewards, formatted with getplural(count, resource)
function format_rewards(r)
   rv = {}
   for name,count in pairs(r) do
      local ware_description = wl.Game():get_ware_description(name)
      -- TRANSLATORS: number + resource name, e.g. '1x Log'
      rv[#rv + 1] = li(_("%1$dx %2$s"):bformat(count, ware_description.descname))
   end
   return table.concat(rv)
end

include "map:scripting/texts.lua"
include "map:scripting/hop_island.lua"
include "map:scripting/first_island.lua"

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

      prefilled_buildings(plr, {"atlanteans_headquarters", sf.x, sf.y,
         wares = {
            diamond = 7,
            iron_ore = 5,
            quartz = 9,
            granite = 50,
            spider_silk = 9,
            log = 20,
            coal = 12,
            gold = 4,
            gold_thread = 6,
            iron = 8,
            planks = 45,
            spidercloth = 5,
            blackroot = 5,
            blackroot_flour = 12,
            atlanteans_bread = 8,
            corn = 5,
            cornmeal = 12,
            fish = 3,
            meat = 3,
            smoked_fish = 6,
            smoked_meat = 6,
            water = 12,
            bread_paddle = 2,
            buckets = 2,
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
            trident_light = 5,
         },
         workers = {
            atlanteans_armorsmith = 1,
            atlanteans_blackroot_farmer = 1,
            atlanteans_builder = 10,
            atlanteans_carrier = 40,
            atlanteans_charcoal_burner = 1,
            atlanteans_fishbreeder = 1,
            atlanteans_geologist = 4,
            atlanteans_miner = 4,
            atlanteans_sawyer = 1,
            atlanteans_stonecutter = 2,
            atlanteans_toolsmith = 2,
            atlanteans_weaponsmith = 1,
            atlanteans_woodcutter = 3,
            atlanteans_horse = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 35,
         }
      })
   end
end

-- Disable some Buildings for all players
function disable_unused_buildings()
   for idx, plr in ipairs(game.players) do
      plr:forbid_buildings{"atlanteans_shipyard"}
   end
end

-- Game initializations
function initialize()
   reveal_everything_for_everybody()
   place_headquarters()
   disable_unused_buildings()

   send_to_all_inboxes(welcome_msg.body, welcome_msg.heading)
   -- set the objective with the game type for all players
   -- TODO change this to a broadcast once individual game objectives have been implemented
   game.players[1]:add_objective("win_conditions", _("Rules"), rt(welcome_msg.body))

   for idx,plr in ipairs(game.players) do
      run(function() run_island(plr, 1) end)
   end

   run(watch_hill)
end


run(initialize)
