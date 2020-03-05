-- =======================================================================
--                 Start conditions for New World
-- =======================================================================

include "scripting/starting_conditions.lua"
include "scripting/ui.lua"

set_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "New World",
   -- TRANSLATORS: This is the tooltip for the "New World" starting condition
   tooltip = _"Start the game with seven ships full of wares on the ocean",
   map_tags = {"seafaring"},

   func = function(player, shared_in_start)

   local map = wl.Game().map
   local sf = map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   local fields = find_ocean_fields(7)
   -- items per expedition (incl. builder): 22
   local items = {
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
   }
   for i,f in pairs(fields) do
      local ship = player:place_ship(f)
      ship:make_expedition(items[i])
   end
   scroll_to_field(fields[1])
end
}

return init
