-- =======================================================================
--                Village starting conditions for Amazons
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

   hq = prefilled_buildings(plr, { "amazons_headquarters", sf.x, sf.y,
      wares = {
         log = 100,
         granite = 50,
         rope = 30,
         liana = 15,
         ironwood = 15,
         rubber = 15,
         balsa = 9,
         water = 20,
         fish = 10,
         meat = 10,
         cassavaroot = 15,
         ration = 5,
         coal = 5,
         gold = 4,
         amazons_bread = 5,
         chocolate = 6,
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
         amazons_carrier = 39,
         amazons_cassava_farmer = 3,
         amazons_cocoa_farmer = 3,
         amazons_stone_carver = 1,
         amazons_geologist = 4,
         amazons_woodcutter = 10,
         amazons_liana_cutter = 3,
         amazons_jungle_preserver = 5,
         amazons_jungle_master = 1,
         amazons_shipwright = 1,
         amazons_hunter_gatherer = 3,
         amazons_stonecutter = 2,
         amazons_trainer = 1,
         amazons_tapir = 5,
         amazons_woodcutter_master = 1,
      },
      soldiers = {
         [{0,0,0,0}] = 5,
      }
   })

      place_building_in_region(plr, "amazons_stone_workshop", sf:region(11), {
         inputs = {
            granite = 6,
            log = 5,
            ironwood = 4,
         },
      })

      place_building_in_region(plr, "amazons_charcoal_kiln", sf:region(10), {
         inputs = {
            log = 6,
         },
      })

      place_building_in_region(plr, "amazons_rope_weaver_booth", sf:region(11), {
         inputs = {
            liana = 6,
         }
      })

end
}

pop_textdomain()
return init
