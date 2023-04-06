-- =======================================================================
--               Discovery starting conditions for Barbarians
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
         log = 7,
         granite = 1,
         barbarians_lumberjack = 3,
         barbarians_ranger = 2,
         barbarians_soldier = 1,
      },
      {
         granite = 1,
         blackwood = 5,
         grout = 3,
         barbarians_stonemason = 2,
         barbarians_gardener = 1,
         barbarians_soldier = 1,
      },
      {
         grout = 1,
         iron = 2,
         barbarians_soldier = 1,
         barbarians_geologist = 1,
         barbarians_miner = 2,
         barbarians_smelter = 2,
         barbarians_blacksmith = 1,
         barbarians_innkeeper = 2,
         barbarians_fisher = 1,
      },
   })
end
}

pop_textdomain()
return init
