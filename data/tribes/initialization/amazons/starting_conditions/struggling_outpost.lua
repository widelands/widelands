-- =======================================================================
--      Hardcore (Struggling Outpost) starting conditions for Amazons
-- =======================================================================

include "scripting/infrastructure.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Struggling Outpost"),
   -- TRANSLATORS: This is the tooltip for the "Hardcore" starting condition
   tooltip = _("Start the game with just your headquarters and very few wares for bootstrapping an economy. Warning: the AI can’t successfully start from this."),
   func =  function(player, shared_in_start)

   local sf = wl.Game().map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   prefilled_buildings(player, { "amazons_headquarters", sf.x, sf.y,
      wares = {
         log = 10,
         rubber = 2,
         rope = 2,
         granite = 3,
      },
      workers = {
         amazons_cook = 2,
         amazons_builder = 2,
         amazons_gold_digger = 1,
         amazons_gold_smelter = 1,
         amazons_carrier = 10,
         amazons_cassava_farmer = 1,
         amazons_cocoa_farmer = 1,
         amazons_stone_carver = 1,
         amazons_geologist = 1,
         amazons_woodcutter = 1,
         amazons_liana_cutter = 2,
         amazons_jungle_preserver = 1,
         amazons_hunter_gatherer = 1,
         amazons_stonecutter = 1,
          amazons_dressmaker = 1,
      },
      soldiers = {
         [{0,0,0,0}] = 2,
      }
   })
end
}

pop_textdomain()
return init
