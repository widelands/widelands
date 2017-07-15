-- =======================================================================
--                 frisians Fortified Village Starting Conditions
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

return {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Fortified Village",
   -- TRANSLATORS: This is the tooltip for the "Fortified Village" starting condition
   tooltip = _"Start the game with a fortified military installation",
   func =  function(plr, shared_in_start)

   local sf = wl.Game().map.player_slots[plr.number].starting_field

      if shared_in_start then
         sf = shared_in_start
      else
         plr:allow_workers("all")
      end

      local h = plr:place_building("frisians_fortress", sf, false, true)
      h:set_soldiers{[{0,0,0,0}] = 10}

      place_building_in_region(plr, "frisians_warehouse", sf:region(7), {
         wares = {
             log = 60,
             granite = 70,
             thatch_reed = 80,
             brick = 90,
             clay = 20,
             water = 10,
             fish = 10,
             meat = 10,
             fruit = 10,
             barley = 5,
             ration = 20,
             honey = 10,
             smoked_meat = 5,
             smoked_fish = 5,
             mead = 5,
             meal = 2,
             coal = 10,
             iron = 5,
             gold = 2,
             iron_ore = 10,
             gold_ore = 2,
             bread_frisians = 10,
             sweetbread = 5,
             beer = 5,
             cloth = 5,
             fur = 10,
             fur_clothes = 5,
             sword_basic = 5,
             hammer = 5,
             fire_tongs = 2,
             bread_paddle = 2,
             kitchen_tools = 2,
             felling_ax = 3,
             needles = 2,
             basket = 2,
             pick = 5,
             shovel = 5,
             scythe = 3,
             hunting_spear = 2,
             fishing_net = 3,
         },
         workers = {
             frisians_blacksmith = 3,
             frisians_baker = 1,
             frisians_brewer = 1,
             frisians_builder = 10,
             frisians_charcoal_burner = 1,
             frisians_clay_burner = 4,
             frisians_carrier = 40,
             frisians_reed_farmer = 2,
             frisians_berry_farmer = 1,
             frisians_farmer = 1,
             frisians_geologist = 4,
             frisians_woodcutter = 3,
             frisians_beekeeper = 1,
             frisians_miner = 4,
             frisians_miner_master = 2,
             frisians_forester = 2,
             frisians_stonemason = 2,
             frisians_reindeer = 5,
         },
         soldiers = {
            [{0,0,0,0}] = 33,
         }
      })

      place_building_in_region(plr, "frisians_training_camp", sf:region(11), {
         inputs = {
            bread_frisians = 4,
            beer = 4,
            smoked_fish = 4,
            smoked_meat = 4,
            sword_long = 1,
            sword_curved = 1,
            sword_double = 1,
            fur_clothes_silver = 1,
            helmet = 1
         },
      })

      place_building_in_region(plr, "frisians_training_arena", sf:region(11), {
         inputs = {
            sweetbread = 4,
            mead = 4,
            smoked_fish = 4,
            smoked_meat = 4,
            sword_long = 1,
            sword_curved = 1,
            sword_double = 1,
            fur_clothes_golden = 1,
            helmet_golden = 1
         },
      })

      place_building_in_region(plr, "frisians_armour_smithy", sf:region(11), {
         inputs = {
            gold = 4,
            coal = 8,
            iron = 8,
         }
      })

      place_building_in_region(plr, "frisians_blacksmithy", sf:region(11), {
         inputs = {
            iron = 8,
            log = 8
         }
      })

      place_building_in_region(plr, "frisians_clay_burners_house", sf:region(11), {
         inputs = {
            water = 2,
         }
      })

      place_building_in_region(plr, "frisians_brick_burners_house", sf:region(11), {
         inputs = {
            clay = 2,
            granite = 2,
            coal = 1
         }
      })

      place_building_in_region(plr, "frisians_reed_farm", sf:region(11))
   end
}
