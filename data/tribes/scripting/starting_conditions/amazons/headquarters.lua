-- =======================================================================
--                 Start conditions for Headquarters
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Headquarters",
   -- TRANSLATORS: This is the tooltip for the "Headquarters" starting condition
   tooltip = _"Start the game with your headquarters only",
   func = function(player, shared_in_start)

   local sf = wl.Game().map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   hq = prefilled_buildings(player, { "amazons_headquarters", sf.x, sf.y,
      wares = {
         log = 100,
         granite = 50,
         rope = 20,
         liana = 15,
         ironwood = 12,
         rubber = 12,
         balsa = 6,
         water = 20,
         fish = 10,
         meat = 10,
         cassavaroot = 15,
         ration = 5,
         coal = 5,
         gold = 4,
         bread_amazons = 5,
         hammer = 3,
         shovel = 5,
         cocoa_beans = 3,
         tunic= 5,
         chisel= 1,


      },
      workers = {
         amazons_cook = 3,
         amazons_builder = 10,
         amazons_gold_digger = 1,
         amazons_gold_smelter = 1,
         amazons_carrier = 40,
         amazons_cassava_farmer = 3,
         amazons_cocoa_farmer = 3,
         amazons_stonecarver = 1,
         amazons_geologist = 4,
         amazons_woodcutter = 10,
         amazons_liana_cutter = 4,
         amazons_jungle_preserver = 5,
         amazons_jungle_master = 10,
         amazons_shipwright = 1,
         amazons_hunter_gatherer = 3,
         amazons_stonecutter = 2,
         amazons_trainer = 3,
         amazons_dressmaker = 1,
         amazons_tapir = 5,
         amazons_woodcutter_master = 1,
      },
      soldiers = {
         [{0,0,0,0}] = 45,
      }
   })
end
}

return init
