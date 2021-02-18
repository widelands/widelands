-- =======================================================================
--               New World starting conditions for Frisians
-- =======================================================================

include "scripting/starting_conditions.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "New World",
   -- TRANSLATORS: This is the tooltip for the "New World" starting condition
   tooltip = _"Start the game with seven ships full of wares on the ocean",
   map_tags = {"seafaring"},
   incompatible_wc = {"scripting/win_conditions/hq_hunter.lua"},

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
         log = 1,
         brick = 2,
         granite = 2,
         reed = 3,
         frisians_claydigger = 1,
         frisians_soldier = 1,
         frisians_brickmaker = 1,
      },
      {
         brick = 1,
         iron = 6,
         frisians_blacksmith = 1,
         frisians_soldier = 1,
         frisians_geologist = 1,
         frisians_miner = 1,
         frisians_smelter = 1,
      },
      {
         log = 3,
         brick = 4,
         reed = 3,
         frisians_stonemason = 1,
         frisians_reed_farmer = 1,
      },
      {
         brick = 2,
         reed = 2,
         granite = 2,
         iron = 2,
         frisians_soldier = 1,
         frisians_geologist = 1,
         frisians_miner = 1,
         frisians_smelter = 1,
      },
      {
         brick = 2,
         reed = 2,
         granite = 2,
         frisians_woodcutter = 3,
         frisians_forester = 2,
      },
      {
         log = 5,
         brick = 5,
         frisians_stonemason = 1,
      },
      {
         log = 2,
         brick = 2,
         reed = 2,
         granite = 2,
         frisians_smoker = 1,
         frisians_fisher = 1,
         frisians_stonemason = 1,
      },
   })
end
}

pop_textdomain()
return init
