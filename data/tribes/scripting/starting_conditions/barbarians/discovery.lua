-- =======================================================================
--                 Start conditions for Discovery
-- =======================================================================

include "scripting/ui.lua"

set_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Discovery",
   -- TRANSLATORS: This is the tooltip for the "Discovery" starting condition
   tooltip = _"Start the game with a ship on the ocean and only a handful of supplies",
   map_tags = {"seafaring"},

   func = function(player, shared_in_start)

   local map = wl.Game().map
   local sf = map.player_slots[player.number].starting_field
   if shared_in_start then
      sf = shared_in_start
   else
      player:allow_workers("all")
   end

   local field = nil
   repeat
      field = map:get_field(math.random(map.width), math.random(map.height))
      if not field:has_caps("swimmable") then
         field = nil
      else
         local route_found = false
         for i,port in pairs(map.port_spaces) do
            if map:sea_route_exists(field, map:get_field(port.x, port.y)) then
               route_found = true
               break
            end
         end
         if not route_found then field = nil end
      end
   until field
   local ship = player:place_ship(field)
   ship.capacity = 50
   ship:make_expedition({
      log = 7,
      granite = 2,
      blackwood = 2,
      grout = 2,
      iron = 2,

      barbarians_stonemason = 1,
      barbarians_lumberjack = 1,
      barbarians_ranger = 1,
      barbarians_gardener = 1,
      barbarians_geologist = 1,
      barbarians_miner = 2,
      barbarians_smelter = 1,
      barbarians_blacksmith = 1,
      barbarians_innkeeper = 1,
      barbarians_fisher = 1,
      barbarians_soldier = 1,
      -- One builder is contained without listing him explicitely
   })
   scroll_to_field(field)
end
}

return init
