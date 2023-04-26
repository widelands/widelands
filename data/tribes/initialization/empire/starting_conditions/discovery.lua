-- =======================================================================
--              Discovery starting conditions for Empire
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
         log = 3,
         granite = 2,
         planks = 3,
         empire_lumberjack = 3,
         empire_forester = 2,
         empire_soldier = 1,
      },
      {
         log = 2,
         marble = 7,
         granite = 1,
         planks = 1,
         empire_stonemason = 2,
         empire_soldier = 1,
      },
      {
         iron = 2,
         log = 1,
         marble = 1,
         empire_soldier = 1,
         empire_geologist = 1,
         empire_miner = 2,
         empire_smelter = 2,
         empire_toolsmith = 1,
         empire_innkeeper = 2,
         empire_fisher = 1,
      },
   })
end
}

pop_textdomain()
return init
