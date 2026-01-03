include "test/scripting/stable_save.lua"

run(function()
   local ship = create_infrastructure(63, 10, true)

   -- Create enemy port and warship
   prefilled_buildings(p2, { "barbarians_port", 7, 10 })

   local enemy_ship = p2:place_ship(map:get_field(5, 14), "warship")
   enemy_ship.hitpoints = 10000

   -- Make own ship a warship
   ship.type = "warship"
   ship.warship_soldier_capacity = 30
   ship.hitpoints = 40000

   -- Send to port
   sleep(1000)
   ship.destination = p1:get_buildings("barbarians_port")[1].portdock

   -- Wait for soldiers to board
   sleep(1000 * 30)
   assert_equal(30, ship:get_workers("barbarians_soldier"))
   assert_equal(0, enemy_ship:get_workers("barbarians_soldier"))

   -- Fight!
   ship:attack(enemy_ship)

   stable_save(game, "naval_battle", 100 * 1000)

   sleep(1000 * 120)

   -- We should have won
   assert_equal(false, enemy_ship.exists)
   assert_true(ship.exists)
   assert_true(ship.hitpoints > 0)
   assert_equal(30, ship:get_workers("barbarians_soldier"))

   -- Healing in port
   local last_hp = ship.hitpoints
   while last_hp < 50000 do
      sleep(20000)
      assert_true(ship.hitpoints > last_hp)
      last_hp = ship.hitpoints
   end
   assert_equal(50000, ship.hitpoints)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
