-- =======================================================================
--                 Empire Fortified Village Starting Conditions
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

return {
   descname = _ "Fortified Village",
   func =  function(plr, shared_in_start)

   local sf = wl.Game().map.player_slots[plr.number].starting_field

   if shared_in_start then
      sf = shared_in_start
   else
      plr:allow_workers("all")
   end

   local h = plr:place_building("empire_castle", sf, false, true)
   h:set_soldiers{[{0,0,0,0}] = 12}

   if not pcall(function()
      place_building_in_region(plr, "empire_warehouse", sf:region(7), {
         wares = {
            armor_helmet = 2,
            spear_wooden = 5,
            felling_ax = 6,
            bread_paddle = 2,
            basket = 2,
            fire_tongs = 2,
            fishing_rod = 2,
            flour = 4,
            grape = 4,
            hammer = 14,
            hunting_spear = 2,
            iron = 4,
            iron_ore = 5,
            kitchen_tools = 4,
            marble = 25,
            marble_column = 6,
            meal = 4,
            pick = 8,
            ration = 12,
            saw = 2,
            scythe = 5,
            shovel = 6,
            granite = 40,
            log = 29,
            water = 12,
            wheat = 4,
            wine = 8,
            planks = 37,
            wool = 2,
         },
         workers = {
            empire_brewer = 1,
            empire_builder = 10,
            empire_carrier = 39,
            empire_charcoal_burner = 1,
            empire_geologist = 4,
            empire_lumberjack = 3,
            empire_miner = 4,
            empire_stonemason = 1,
            empire_toolsmith = 1,
            empire_donkey = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 33,
         }
      })

      place_building_in_region(plr, "empire_colosseum", sf:region(11), {
         wares = {
            empire_bread = 8,
            fish = 4,
            meat = 4,
         },
      })

      place_building_in_region(plr, "empire_trainingcamp", sf:region(11), {
         wares = {
            fish = 2,
            meat = 2,
            armor_helmet = 2,
         },
      })

      place_building_in_region(plr, "empire_armorsmithy", sf:region(11), {
         wares = {
               gold = 4,
               coal = 8,
               cloth = 5,
         }
      })

      place_building_in_region(plr, "empire_toolsmithy", sf:region(11), {
         wares = {
            iron = 8,
         }
      })

      place_building_in_region(plr, "empire_weaponsmithy", sf:region(11), {
         wares = {
            coal = 4,
            planks = 8,
         }
      })

      place_building_in_region(plr, "empire_sawmill", sf:region(11), {
         wares = {
            log = 1,
         }
      })

      place_building_in_region(plr, "empire_stonemasons_house", sf:region(11))
   end) then
      plr:send_message(
			-- TRANSLATORS: Short for "Not enough space"
			_"Space",
			rt(p(_([[Some of your starting buildings didn’t have enough room and weren’t built. You are at a disadvantage with this; consider restarting this map with a fair starting condition.]]))),
			{popup=true, heading=_"Not enough space"}
      )
   end
end
}
