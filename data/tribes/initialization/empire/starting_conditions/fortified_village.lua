-- =======================================================================
--            Fortified Village starting conditions for Empire
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Fortified Village"),
   -- TRANSLATORS: This is the tooltip for the "Fortified Village" starting condition
   tooltip = _("Start the game with a fortified military installation"),
   incompatible_wc = {"scripting/win_conditions/hq_hunter.lua"},

   func =  function(plr, shared_in_start)

   local sf = wl.Game().map.player_slots[plr.number].starting_field

      if shared_in_start then
         sf = shared_in_start
      else
         plr:allow_workers("all")
      end

      local h = plr:place_building("empire_castle", sf, false, true)
      h:set_soldiers{[{0,0,0,0}] = 12}

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
            empire_trainer = 3,
            empire_donkey = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 33,
         }
      })

      place_building_in_region(plr, "empire_colosseum", sf:region(11), {
         inputs = {
            empire_bread = 8,
            fish = 4,
            meat = 4,
         },
      })

      place_building_in_region(plr, "empire_trainingcamp", sf:region(11), {
         inputs = {
            fish = 2,
            meat = 2,
            armor_helmet = 2,
         },
      })

      place_building_in_region(plr, "empire_armorsmithy", sf:region(11), {
         inputs = {
               gold = 4,
               coal = 8,
               cloth = 5,
         }
      })

      place_building_in_region(plr, "empire_toolsmithy", sf:region(11), {
         inputs = {
            iron = 8,
         }
      })

      place_building_in_region(plr, "empire_weaponsmithy", sf:region(11), {
         inputs = {
            coal = 4,
            planks = 8,
         }
      })

      place_building_in_region(plr, "empire_sawmill", sf:region(11), {
         inputs = {
            log = 1,
         }
      })

      place_building_in_region(plr, "empire_stonemasons_house", sf:region(11))
   end
}
pop_textdomain()
return init
