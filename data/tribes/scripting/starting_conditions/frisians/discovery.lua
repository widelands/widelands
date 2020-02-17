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

   -- items per expedition (incl. builder): 25
   local items = {
      {
         granite = 1,
         brick = 3,
         reed = 5,
         log = 4,
         frisians_soldier = 1,
         frisians_brickmaker = 1,
         frisians_reed_farmer = 1,
      },
      {
         granite = 1,
         brick = 3,
         reed = 3,
         log = 2,
         frisians_soldier = 1,
         frisians_stonemason = 2,
         frisians_woodcutter = 3,
         frisians_forester = 2,
      },
      {
         iron = 2,
         reed = 2,
         brick = 3,
         frisians_claydigger = 1,
         frisians_soldier = 1,
         frisians_geologist = 1,
         frisians_miner = 2,
         frisians_smelter = 2,
         frisians_blacksmith = 1,
         frisians_smoker = 1,
         frisians_fisher = 1,
      },
   }
   for i,f in pairs(fields) do
      local ship = player:place_ship(f)
      local mincap = #items[i] + 25
      if mincap > ship.capacity then ship.capacity = mincap end
      ship:make_expedition(items[i])
   end
   scroll_to_field(fields[1])
end
}

return init
