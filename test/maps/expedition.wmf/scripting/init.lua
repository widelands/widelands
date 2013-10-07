use("aux", "lunit")
use("aux", "coroutine")
use("aux", "ui")
lunit.import "assertions"

-- This is a test case for bug 1234058: there is constant demand for trunks,
-- so the expedition initially never got any.
-- We also exercise the expedition feature and send the ship around, build a
-- port and construct a building in the colony.

game = wl.Game()
map = game.map
p1 = game.players[1]
mv = wl.ui.MapView()

p1:allow_workers("all")
prefilled_buildings(p1,
{ "headquarters", 8, 18,
   wares = {
      blackwood = 3,
      grout = 2,
      iron = 2,
      raw_stone = 5,
      thatchreed = 4,
      trunk = 3,
   },
   workers = {
      builder = 1,
   },
},
-- But some wares needed for the expedition into the port.
{ "port", 16, 16,
   wares = {
      gold = 2,
   },
   workers = {
   },
   soldiers = {
   }
},
-- We need a shipyard, because there is no way to make ships via scripting yet.
{ "shipyard", 10, 15,
   wares = {
      blackwood = 10,
      trunk = 2,
      cloth = 4,
   },
   workers = {
      shipwright = 1,
   }
},
{ "rangers_hut", 5, 20, workers = { ranger = 1 } },
{ "rangers_hut", 4, 10, workers = { ranger = 1 } },
{ "lumberjacks_hut", 7, 20, workers = { lumberjack = 1 } },
{ "hardener", 10, 18, workers = { lumberjack = 1 } }
)

hq = map:get_field(8, 18).immovable

connected_road(p1, map:get_field(8,19).immovable, "r,r|r,r|r,r|r,tr,tr|", true)
connected_road(p1, map:get_field(7,21).immovable, "tr,tr|", true)

-- This keeps eating blackwood, so that the burner will never stop working.
run(function()
   while true do
      hq:set_wares("blackwood", 0)
      sleep(1000)
   end
end)

-- Main logic.
run(function()
   sleep(100)

   game.desired_speed = 1000 * 1000

   -- Start the expedition
   mouse_smoothly_to(map:get_field(16, 16), 500)
   mv:click(map:get_field(16, 16))
   mv.windows.building_window.buttons.start_expedition:click()
   mv.windows.building_window:close()

   -- Wait till the expedition is ready (we need to build a ship).
   sleep(60 * 1000 * 20)
   assert_equal(p1.inbox[#p1.inbox].title, "Expedition ready")

   -- Now cancel the expedition before it even got started.
   -- sleep(1000)
   -- mouse_smoothly_to(map:get_field(14, 13), 0)
   -- mv:click(map:get_field(14, 13)) -- click on the ship
   -- mv.windows.shipwindow.buttons.cancel_expedition:click()
   -- mv.windows.ship_action_confirm.buttons.ok:click()

   game.desired_speed = 1000



   print("# All Tests passed.")

   -- mv:close()
end)
