-- =======================================================================
--                 Discovery starting conditions for Amazons
-- =======================================================================

include "scripting/starting_conditions.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("Discovery"),
   -- TRANSLATORS: This is the tooltip for the "Discovery" starting condition
   tooltip = _("Start the game with three ships on the ocean and only a handful of supplies"),
   map_tags = {"seafaring"},
   incompatible_wc = {"scripting/win_conditions/hq_hunter.lua"},
   uses_map_starting_position = false,

   func = function(player, shared_in_start)

   local map = wl.Game().map
   local sf = map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   launch_expeditions(player, {
      {
         log = 4,
         granite = 1,
         rope = 2,
         rubber = 3,
         amazons_woodcutter = 3,
         amazons_jungle_preserver = 2,
         amazons_soldier = 1,
      },
      {
         log = 6,
         granite = 4,
         rope = 2,
         amazons_stonecutter = 2,
         amazons_soldier = 1,
      },
      {
         granite = 2,
         amazons_soldier = 1,
         amazons_geologist = 1,
         amazons_gold_digger = 1,
         amazons_gold_smelter = 1,
         amazons_stone_carver = 2,
         amazons_liana_cutter = 2,
         amazons_cassava_farmer = 1,
         amazons_cook = 2,
         amazons_hunter_gatherer = 1,
         amazons_woodcutter = 1,
      },
   })
end
}

pop_textdomain()
return init
