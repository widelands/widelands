-- =======================================================================
--           New World starting conditions for Atlanteans
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
         log = 4,
         granite = 2,
         planks = 2,
         iron = 1,
         atlanteans_soldier = 1,
      },
      {
         granite = 2,
         planks = 2,
         iron = 1,
         atlanteans_soldier = 1,
         atlanteans_geologist = 1,
         atlanteans_miner = 2,
      },
      {
         spidercloth = 1,
         atlanteans_soldier = 1,
         atlanteans_stonecutter = 1,
         iron = 4,
         atlanteans_smelter = 2,
         atlanteans_toolsmith = 1,
      },
      {
         log = 6,
         granite = 2,
         atlanteans_stonecutter = 1,
      },
      {
         log = 4,
         iron = 1,
         atlanteans_geologist = 1,
         atlanteans_miner = 3,
      },
      {
         log = 1,
         granite = 2,
         planks = 2,
         atlanteans_woodcutter = 3,
         atlanteans_forester = 2,
      },
      {
         granite = 2,
         planks = 2,
         atlanteans_stonecutter = 1,
         atlanteans_baker = 1,
         atlanteans_smoker = 2,
         atlanteans_fisher = 1,
      },
   })
end
}

pop_textdomain()
return init
