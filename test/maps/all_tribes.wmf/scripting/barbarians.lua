-- ===============
-- Initialization
-- ===============

function init_barbarians()

   local player = wl.Game().players[1]

   local tribe = player.tribe
   local sf = wl.Game().map.player_slots[player.number].starting_field
   map = wl.Game().map

   place_warehouse(player, sf, tribe, "barbarians_headquarters")
   place_initial_militarysites(map, sf, player, "barbarians_fortress")
end
