-- =======================================================================
--             Discovery starting conditions for Frisians
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
         granite = 1,
         brick = 3,
         reed = 5,
         log = 4,
         frisians_soldier = 1,
         frisians_brickmaker = 1,
         frisians_reed_farmer = 1,
      },
      {
         granite = 1,
         brick = 3,
         reed = 3,
         log = 2,
         frisians_soldier = 1,
         frisians_stonemason = 2,
         frisians_woodcutter = 3,
         frisians_forester = 2,
      },
      {
         iron = 2,
         reed = 2,
         brick = 3,
         frisians_claydigger = 1,
         frisians_soldier = 1,
         frisians_geologist = 1,
         frisians_miner = 2,
         frisians_smelter = 2,
         frisians_blacksmith = 1,
         frisians_smoker = 1,
         frisians_fisher = 1,
      },
   })
end
}

pop_textdomain()
return init
