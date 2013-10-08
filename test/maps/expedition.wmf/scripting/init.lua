use("aux", "lunit")
use("aux", "coroutine")
use("aux", "ui")
lunit.import "assertions"

-- This is a test case for bug 1234058: there is constant demand for trunks,
-- so the expedition initially never got any.

game = wl.Game()
map = game.map
p1 = game.players[1]
mv = wl.ui.MapView()

p1:allow_workers("all")
prefilled_buildings(p1,
{ "headquarters", 8, 18,
   wares = {
      blackwood = 3,
      gold = 2,
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

local port = prefilled_buildings(p1, { "port", 16, 16,
   wares = {
   },
   workers = {
   },
   soldiers = {
   }
})

-- This keeps eating blackwood.
run(function()
   while true do
      hq:set_wares("blackwood", 0)
      sleep(1000)
   end
end)

run(function()
   sleep(100)
   -- game.desired_speed = 150000

   mouse_smoothly_to(map:get_field(16, 16), 0)
   mv:click(map:get_field(16, 16))
   mv.windows.building_window.buttons.start_expedition:click()
   mv.windows.building_window:close()

   sleep(60 * 1000 * 20)




   lunit:run()
   -- mv:close()
end)
