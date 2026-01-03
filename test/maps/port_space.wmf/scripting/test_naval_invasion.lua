run(function()
   -- We need a slightly larger island for this testcase
   local portspace = map:get_field(7, 10)
   local milsitespace = portspace.bln.bln.bln.bln
   milsitespace.tln.terr = "steppe"
   milsitespace.tln.terd = "steppe"
   milsitespace.terd = "steppe"
   milsitespace.terr = "steppe"
   milsitespace.ln.terr = "steppe"
   milsitespace.trn.terd = "steppe"
   milsitespace.trn.trn.terr = "steppe"
   milsitespace.ln.ln.bln.terr = "summer_water"
   milsitespace.ln.bln.bln.terr = "summer_water"
   milsitespace.ln.ln.ln.terr = "summer_water"
   milsitespace.ln.ln.tln.terd = "summer_water"

   local ship = create_infrastructure(milsitespace.x, milsitespace.y, true)

   -- Create enemy port and a militarysite
   prefilled_buildings(p2, { "barbarians_port", portspace.x, portspace.y, soldiers = {[{0,0,0,0}] = 4 }})

   local enemy_port = portspace.immovable
   local enemy_tower = milsitespace.immovable
   enemy_tower.capacity = 1
   connected_road("normal", p2, enemy_port.flag, "bl,bl,bl,bl")

   -- Make own ship a warship
   ship.type = "warship"
   ship.warship_soldier_capacity = 20
   ship.hitpoints = 50000

   -- Send to port
   sleep(1000)
   ship.destination = p1:get_buildings("barbarians_port")[1].portdock

   -- Wait for soldiers to board
   sleep(1000 * 30)
   assert_equal(20, ship:get_workers("barbarians_soldier"))
   assert_equal(4, enemy_port:get_workers("barbarians_soldier"))

   -- Fight!
   ship.destination = nil
   local soldiers = ship:get_workers("")
   assert_equal(20, #soldiers)
   for i = 6, 20 do soldiers[i] = nil end
   assert_equal(5, #soldiers)
   ship:invade(portspace, soldiers)

   sleep(1000 * 120)

   -- We should have won
   assert_equal(false, enemy_port.exists)
   assert_equal(false, enemy_tower.exists)
   assert_true(ship.exists)
   assert_true(ship.hitpoints == 50000)
   assert_equal(15, ship:get_workers("barbarians_soldier"))
   assert_equal(portspace.owner, p1)
   assert_equal(milsitespace.owner, p1)
   local new_tower = milsitespace.immovable
   assert_not_nil(new_tower)
   assert_equal(new_tower.owner, p1)
   assert_true(new_tower:get_soldiers("associated") >= 1)
   assert_true(new_tower:get_soldiers("associated") <= 5)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
