-- =======================================================================
--                    Village Starting Conditions
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

return {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Village",
   -- TRANSLATORS: This is the tooltip for the "Village" starting condition
   tooltip = _"Start the game with a peaceful village",
   func =  function(plr, shared_in_start)

      local sf = wl.Game().map.player_slots[plr.number].starting_field
      if shared_in_start then
         sf = shared_in_start
      else
         plr:allow_workers("all")
      end

   prefilled_buildings(plr, { "europeans_headquarters", sf.x, sf.y,
         wares = {
            water = 128,
            log = 64,
            planks = 40,
            spidercloth = 40,
            granite = 64,
            reed = 32,
            grout = 32,
            brick = 32,
            marble = 16,
            diamond = 8,
            quartz = 8,
            ore = 16,
            coal = 64,
            iron = 8,
            basket = 10,
            hammer = 10,
            saw = 10,
            shovel = 9,
            pick = 8,
            fire_tongs = 2,
            buckets = 4,
            scythe = 4,
         },
         workers = {
            europeans_builder = 10,
            europeans_carrier = 38,
            europeans_geologist = 4,
            europeans_miner = 4,
            europeans_stonecutter = 2,
            europeans_stonemason = 2,
            europeans_toolsmith = 2,
            europeans_trainer = 3,
            europeans_lumberjack = 3,
            europeans_woodcutter = 3,
            europeans_ranger = 3,
            europeans_forester = 3,
            europeans_sawyer = 2,
            europeans_fisher = 2,
            europeans_hunter = 2,
            europeans_smoker = 1,
            europeans_miller = 1,
            europeans_baker = 1,
            europeans_spiderbreeder = 1,
            europeans_weaver = 1,
         },
         soldiers = {
            [{0,0,0,0}] = 5,
         },
      })

      place_building_in_region(plr, "europeans_well_basic", sf:region(4), {
      })
      place_building_in_region(plr, "europeans_sawmill_basic", sf:region(6), {
         inputs = {
            log = 6
         }
      })
      place_building_in_region(plr, "europeans_stonemasons_hut", sf:region(6), {
         inputs = {
            clay = 6,
            granite = 6,
            water = 6
         }
      })
      place_building_in_region(plr, "europeans_smelting_works_basic", sf:region(8), {
         inputs = {
            coal = 6,
            ore = 6
         }
      })
      place_building_in_region(plr, "europeans_toolsmithy", sf:region(8), {
         inputs = {
            planks = 4,
            iron = 4,
            spidercloth = 4
         }
      })
      place_building_in_region(plr, "europeans_weaving_mill_basic", sf:region(8), {
         inputs = {
         spider_silk = 6 ,
         wool = 6
      },
      })


   end
}
