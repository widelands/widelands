-- =======================================================================
--                 Start conditions for Discovery
-- =======================================================================

include "scripting/starting_conditions.lua"

set_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Discovery",
   -- TRANSLATORS: This is the tooltip for the "Discovery" starting condition
   tooltip = _"Start the game with three ships on the ocean and only a handful of supplies",
   map_tags = {"seafaring"},

   func = function(player, shared_in_start)

   local map = wl.Game().map
   local sf = map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   local fields = find_ocean_fields(3)
   -- items per expedition (incl. builder): 18
   local items = {
      {
         log = 4,
         granite = 1,
         planks = 2,
         spidercloth = 3,
         atlanteans_woodcutter = 3,
         atlanteans_forester = 2,
         atlanteans_soldier = 1,
      },
      {
         log = 3,
         granite = 4,
         planks = 2,
         spidercloth = 3,
         atlanteans_stonecutter = 2,
         atlanteans_soldier = 1,
      },
      {
         iron = 2,
         atlanteans_soldier = 1,
         atlanteans_geologist = 1,
         atlanteans_miner = 3,
         atlanteans_smelter = 2,
         atlanteans_toolsmith = 1,
         atlanteans_baker = 2,
         atlanteans_smoker = 2,
         atlanteans_fisher = 1,
      },
   }
   for i,f in pairs(fields) do
      local ship = player:place_ship(f)
      if i == 1 then ship.capacity = 46 else ship.capacity = 45 end
      ship:make_expedition(items[i])
   end
   sleep_then_goto(1000, fields[1])
end
}

return init
