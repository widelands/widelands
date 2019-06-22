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
      if not field:has_caps("swimmable") then field = nil end
      -- NOCOM check whether we are on an ocean with a port space!
   until field
   local ship = player:place_ship(field)
   ship.capacity = 50
   ship:make_expedition({
      log = 7,
      granite = 5,
      planks = 4,
      spidercloth = 1,
      iron = 2,

      atlanteans_stonecutter = 1,
      atlanteans_woodcutter = 1,
      atlanteans_forester = 1,
      atlanteans_geologist = 1,
      atlanteans_miner = 3,
      atlanteans_smelter = 1,
      atlanteans_toolsmith = 1,
      atlanteans_baker = 1,
      atlanteans_smoker = 1,
      atlanteans_fisher = 1,
      atlanteans_soldier = 1,
      -- One builder is contained without listing him explicitely
   })
   scroll_to_field(field)
end
}

return init
