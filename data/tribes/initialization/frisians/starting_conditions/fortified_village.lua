-- =======================================================================
--            Fortified Village starting conditions for Frisians
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

      local h = plr:place_building("frisians_fortress", sf, false, true)
      h:set_soldiers{[{0,0,0,0}] = 10}

      place_building_in_region(plr, "frisians_warehouse", sf:region(7), {
         wares = {
            log = 25,
            granite = 35,
            reed = 20,
            brick = 50,
            clay = 30,
            water = 10,
            fish = 10,
            meat = 10,
            fruit = 10,
            barley = 5,
            ration = 20,
            honey = 10,
            meal = 2,
            coal = 10,
            iron_ore = 10,
            cloth = 5,
            fur = 10,
            fur_garment = 5,
            sword_short = 5,
            hammer = 10,
            fire_tongs = 2,
            bread_paddle = 2,
            kitchen_tools = 2,
            felling_ax = 3,
            needles = 1,
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
             frisians_claydigger = 2,
             frisians_brickmaker = 2,
             frisians_carrier = 40,
             frisians_reed_farmer = 2,
             frisians_berry_farmer = 1,
             frisians_farmer = 1,
             frisians_landlady = 1,
             frisians_smoker = 1,
             frisians_geologist = 4,
             frisians_woodcutter = 3,
             frisians_beekeeper = 1,
             frisians_miner = 4,
             frisians_miner_master = 2,
             frisians_forester = 2,
             frisians_stonemason = 2,
             frisians_reindeer = 5,
             frisians_trainer = 3,
             frisians_seamstress_master = 1,
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
            sword_broad = 1,
            sword_double = 1,
            fur_garment_studded = 1,
            helmet = 1
         },
      })

      place_building_in_region(plr, "frisians_training_arena", sf:region(11), {
         inputs = {
            honey_bread = 4,
            mead = 4,
            smoked_fish = 4,
            smoked_meat = 4,
            sword_long = 1,
            sword_broad = 1,
            sword_double = 1,
            fur_garment_golden = 1,
            helmet_golden = 1
         },
      })

      place_building_in_region(plr, "frisians_armor_smithy_small", sf:region(11), {
         inputs = {
            coal = 6,
            iron = 6,
         }
      })

      place_building_in_region(plr, "frisians_armor_smithy_large", sf:region(11), {
         inputs = {
            gold = 4,
            coal = 4,
            iron = 4,
         }
      })

      place_building_in_region(plr, "frisians_blacksmithy", sf:region(11), {
         inputs = {
            iron = 6,
            log = 6,
            reed = 4,
         }
      })

      place_building_in_region(plr, "frisians_clay_pit", sf:region(11), {
         inputs = {
            water = 2,
         }
      })

      place_building_in_region(plr, "frisians_brick_kiln", sf:region(11), {
         inputs = {
            clay = 2,
            granite = 2,
            coal = 1
         }
      })

      place_building_in_region(plr, "frisians_reed_farm", sf:region(11))
   end
}
pop_textdomain()
return init
