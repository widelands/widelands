-- =======================================================================
--                 Village starting conditions for Empire
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Village"),
   -- TRANSLATORS: This is the tooltip for the "Village" starting condition
   tooltip = _("Start the game with a peaceful village"),
   func = function(plr, shared_in_start)

   local sf = wl.Game().map.player_slots[plr.number].starting_field

      if shared_in_start then
         sf = shared_in_start
      else
         plr:allow_workers("all")
      end

prefilled_buildings(plr, { "empire_headquarters", sf.x, sf.y,
       wares = {
            armor_helmet = 3,
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
            iron = 9,
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
            log = 29,
            water = 12,
            wheat = 4,
            wine = 8,
            planks = 45,
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
            empire_trainer = 1,
            empire_donkey = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 5,
         }
      })

      place_building_in_region(plr, "empire_toolsmithy", sf:region(9), {
         inputs = {log = 7, iron = 7
         }
      })

      place_building_in_region(plr, "empire_sawmill", sf:region(9), {
         inputs = {log = 7
         }
      })

      place_building_in_region(plr, "empire_stonemasons_house", sf:region(9), {
      inputs = {marble = 6 }
   })
   end
}
pop_textdomain()
return init
