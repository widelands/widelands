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
   -- items per expedition (incl. builder): 22
   local items = {
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
   }
   for i,f in pairs(fields) do
      local ship = player:place_ship(f)
      if i == 1 then ship.capacity = 36 else ship.capacity = 35 end
      ship:make_expedition(items[i])
   end
   sleep_then_goto(1000, fields[1])
end
}

return init
