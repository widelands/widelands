-- =======================================================================
--              New World starting conditions for Barbarians
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
         blackwood = 2,
         granite = 1,
         barbarians_soldier = 1,
         barbarians_gardener = 1,
      },
      {
         log = 3,
         blackwood = 2,
         barbarians_soldier = 1,
         barbarians_stonemason = 1,
      },
      {
         blackwood = 2,
         granite = 1,
         grout = 1,
         barbarians_soldier = 1,
         barbarians_geologist = 1,
         barbarians_miner = 1,
         barbarians_smelter = 1,
      },
      {
         barbarians_geologist = 1,
         barbarians_miner = 1,
         barbarians_smelter = 1,
         iron = 4,
         barbarians_blacksmith = 1,
      },
      {
         log = 7,
         barbarians_stonemason = 1,
      },
      {
         log = 1,
         blackwood = 2,
         granite = 1,
         grout = 1,
         barbarians_stonemason = 1,
         barbarians_innkeeper = 1,
         barbarians_fisher = 1,
      },
      {
         log = 1,
         granite = 1,
         barbarians_lumberjack = 3,
         barbarians_ranger = 2,
      },
   })
end
}

pop_textdomain()
return init
