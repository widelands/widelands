run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_southern_port()
   create_northern_port()

   --removing portdock first
   portdock_fields=northern_port().portdock.fields
   portdock_fields[1].immovable:remove()
   sleep(100)
   --portdock should be back, as port is still there
   assert (portdock_fields[1].immovable)

   start_building_farm()
   southern_port():set_workers{
      barbarians_builder = 1,
   }

   sleep(2000)

   -- The ship should not yet have picked up the worker from the
   -- portdock.
   assert_equal(1, p1:get_workers("barbarians_builder"))
   assert_equal(0, southern_port():get_workers("barbarians_builder"))

  -- so wait till builder is loaded on ship
  while (ship:get_workers("barbarians_builder") == 0) do
      sleep(100)
  end
  sleep(5000)

   portdock_fields=northern_port().portdock.fields
   northern_port():remove()
   sleep(100)
   --verify that also portdock was removed
   assert (not portdock_fields[1].immovable)

   sleep(100)

   stable_save("worker_in_portdock")

   -- Wait till a ship unloads a worked at the souther port
   while (ship:get_workers("builder") == 1) do
       sleep(100)
   end
   sleep(2000)

   -- Worker should be back in the southern port.
   assert_equal(p1:get_workers("barbarians_builder"), 1)
   assert_equal(southern_port():get_workers("barbarians_builder"), 1)

   -- Create port again.
   create_northern_port()

   while ship:get_workers() == 0 do
      sleep(50)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
