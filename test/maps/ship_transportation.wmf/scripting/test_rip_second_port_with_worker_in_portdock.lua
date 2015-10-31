run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_first_port()
   create_second_port()

   --removing portdock first
   portdock_fields=port2().portdock.fields
   portdock_fields[1].immovable:remove()
   sleep(100)
   --portdock should be back, as port is still there
   assert (portdock_fields[1].immovable)

   start_building_farm()
   port1():set_workers{
      barbarians_builder = 1,
   }
   sleep(6000)

   -- The ship should not yet have picked up the worker from the
   -- portdock.
   assert_equal(p1:get_workers("barbarians_builder"), 1)
   assert_equal(port1():get_workers("barbarians_builder"), 0)

   portdock_fields=port2().portdock.fields
   port2():remove()
   sleep(100)
   --verify that also portdock was removed
   assert (not portdock_fields[1].immovable)

   sleep(100)

   stable_save("worker_in_portdock")

   sleep(8000)

   -- Worker should be back in port.
   assert_equal(p1:get_workers("barbarians_builder"), 1)
   assert_equal(port1():get_workers("barbarians_builder"), 1)

   -- Create port again.
   create_second_port()

   while ship:get_workers() == 0 do
      sleep(50)
   end

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
