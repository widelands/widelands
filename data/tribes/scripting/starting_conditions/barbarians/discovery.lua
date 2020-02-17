-- =======================================================================
--                 Start conditions for Discovery
-- =======================================================================

include "scripting/ui.lua"

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

   local fields = {}
   repeat
      local f = map:get_field(math.random(map.width), math.random(map.height))
      if not f:has_caps("swimmable") then
         f = nil
      else
         local route_found = false
         for i,port in pairs(map.port_spaces) do
            if map:sea_route_exists(f, map:get_field(port.x, port.y)) then
               route_found = true
               break
            end
         end
         if not route_found then f = nil end
      end
      if f then table.insert(fields, f) end
   until #fields == 3

   -- default capacity:                     32 (!)
   -- items per expedition (incl. builder): 22
   -- free capacity per ship:               10 ×3
   local items = {
      {
         log = 3,
         granite = 1,
         grout = 3,
         barbarians_stonemason = 1,
         barbarians_lumberjack = 1,
         barbarians_ranger = 1,
      },
      {
         log = 1,
         iron = 2,
         barbarians_innkeeper = 1,
         barbarians_fisher = 1,
         barbarians_geologist = 1,
         barbarians_miner = 2,
         barbarians_smelter = 1,
         barbarians_blacksmith = 1,
      },
      {
         log = 3,
         granite = 1,
         grout = 1,
         blackwood = 1,
         reed = 2
         barbarians_gardener = 1,
         barbarians_soldier = 1,
      },
   }
   for i,f in pairs(fields) do
      local ship = player:place_ship(f)
      ship.capacity = 32
      ship:make_expedition(items[i])
   end
   scroll_to_field(fields[1])
end
}

return init
