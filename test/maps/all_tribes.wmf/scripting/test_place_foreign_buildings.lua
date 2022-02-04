-- This script tests placing of buildings not owned by the tribe.
-- Another player with the tribe exists.

-- Run the tests
run(function()
   sleep(1000)
   local playernumber = 1
   local player = game.players[playernumber]

   print("Placing buildings for Player 1")
   assert_equal("barbarians", player.tribe.name)

   local sf = wl.Game().map.player_slots[playernumber].starting_field
   map = wl.Game().map

   -- Headquarters
   place_warehouse(player, "barbarians_headquarters", sf, 0, 0)

   -- Somebody else's militarysites are OK
   place_initial_militarysites(map, sf, player, "frisians_fortress")

   -- Other buildings are not OK
   assert_error("Should not be able to place a building that the tribe can't use", function()
      place_safe_building(player, "empire_port", sf, 11, 509)
   end)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
