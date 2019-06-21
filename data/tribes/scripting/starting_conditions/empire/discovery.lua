-- =======================================================================
--                 Start conditions for Discovery
-- =======================================================================

include "scripting/infrastructure.lua"

set_textdomain("tribes")

init = {
   -- TRANSLATORS: This is the name of a starting condition
   descname = _ "Discovery",
   -- TRANSLATORS: This is the tooltip for the "Discovery" starting condition
   tooltip = _"Start the game with a ship on the ocean and a handful of supplies",
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
      if not field:has_caps("swimmable") then field = nil end
      -- NOCOM check whether we are on an ocean with a port space!
   until field
   local ship = player:place_ship(field)
   ship.capacity = 50
   ship:make_expedition({
      log = 6,
      granite = 3,
      planks = 4,
      marble = 4,
      iron = 2,

      empire_stonemason = 1,
      empire_lumberjack = 1,
      empire_forester = 1,
      empire_geologist = 1,
      empire_miner = 2,
      empire_smelter = 1,
      empire_toolsmith = 1,
      empire_innkeeper = 1,
      empire_fisher = 1,
      empire_soldier = 1,
      -- One builder is contained without listing him explicitely
   })
end
}

return init
