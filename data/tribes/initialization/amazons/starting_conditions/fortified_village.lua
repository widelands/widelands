-- =======================================================================
--             Fortified Village starting conditions for Amazons
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

      local h = plr:place_building("amazons_fortification", sf, false, true)
      h:set_soldiers{[{0,0,0,0}] = 12}

      place_building_in_region(plr, "amazons_warehouse", sf:region(7), {
         wares = {
         log = 86,
         granite = 43,
         rope = 24,
         liana = 7,
         ironwood = 8,
         rubber = 6,
         balsa = 3,
         water = 20,
         cassavaroot = 15,
         ration = 5,
         coal = 5,
         gold = 1,
         hammer = 10,
         shovel = 5,
         cocoa_beans = 3,
         tunic = 5,
         chisel = 3,
         pick = 2,
         machete = 2,
         spear_wooden = 6,
         stone_bowl = 2,
         needles = 2,
         },
         workers = {
            amazons_cook = 3,
            amazons_builder = 10,
            amazons_gold_digger = 1,
            amazons_gold_smelter = 1,
            amazons_carrier = 38,
            amazons_cassava_farmer = 3,
            amazons_cocoa_farmer = 3,
            amazons_stone_carver = 2,
            amazons_geologist = 4,
            amazons_woodcutter = 10,
            amazons_liana_cutter = 3,
            amazons_jungle_preserver = 5,
            amazons_jungle_master = 3,
            amazons_shipwright = 1,
            amazons_hunter_gatherer = 3,
            amazons_stonecutter = 2,
            amazons_trainer = 3,
            amazons_tapir= 5,
            amazons_woodcutter_master = 3,
         },
         soldiers = {
            [{0,0,0,0}] = 33,
         }
      })

      place_building_in_region(plr, "amazons_training_glade", sf:region(11), {
         inputs = {
            amazons_bread = 3,
            fish = 5,
            meat = 5,
            chocolate = 3,
         },
      })

      place_building_in_region(plr, "amazons_warriors_gathering", sf:region(11), {
         inputs = {
            amazons_bread = 2,
            fish = 4,
            meat = 4,
            chocolate = 3,
         },
      })

      place_building_in_region(plr, "amazons_dressmakery", sf:region(11), {
         inputs = {
               ironwood = 2,
               balsa = 6,
               rubber = 9,
               rope = 6,
               gold = 3,
         }
      })

      place_building_in_region(plr, "amazons_stone_workshop", sf:region(11), {
         inputs = {
            granite = 7,
            log = 6,
            ironwood = 5,
         }
      })

      place_building_in_region(plr, "amazons_charcoal_kiln", sf:region(11), {
         inputs = {
            log = 8,
         }
      })

      place_building_in_region(plr, "amazons_rope_weaver_booth", sf:region(11), {
         inputs = {
            liana = 8,
         }
      })
   end
}

pop_textdomain()
return init
