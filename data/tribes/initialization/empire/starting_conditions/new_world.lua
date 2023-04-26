-- =======================================================================
--             New World starting conditions for Empire
-- =======================================================================

include "scripting/starting_conditions.lua"

push_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _("New World"),
   -- TRANSLATORS: This is the tooltip for the "New World" starting condition
   tooltip = _("Start the game with seven ships full of wares on the ocean"),
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
         log = 3,
         granite = 2,
         planks = 2,
         empire_soldier = 1,
         empire_stonemason = 1,
      },
      {
         log = 6,
         marble = 2,
         empire_soldier = 1,
      },
      {
         iron = 4,
         empire_toolsmith = 1,
         empire_soldier = 1,
         empire_geologist = 1,
         empire_miner = 1,
         empire_smelter = 1,
      },
      {
         marble = 2,
         granite = 2,
         planks = 2,
         empire_geologist = 1,
         empire_miner = 1,
         empire_smelter = 1,
      },
      {
         granite = 2,
         planks = 2,
         empire_lumberjack = 3,
         empire_forester = 2,
      },
      {
         marble = 2,
         granite = 2,
         planks = 2,
         empire_stonemason = 1,
         empire_innkeeper = 1,
         empire_fisher = 1,
      },
      {
         log = 6,
         marble = 2,
         empire_stonemason = 1,
      },
   })
end
}

pop_textdomain()
return init
