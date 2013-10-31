run(function()
   sleep(100)
   -- game.desired_speed = 10 * 1000

   create_first_port()
   create_second_port()

   sleep(1000)
   start_building_farm()
   port1:set_workers{
      builder = 1,
   }
   sleep(10000)
   -- NOCOM(#sirver): what if the first port is deleted?
   -- NOCOM(#sirver): what about expeditions?
   -- NOCOM(#sirver): what if the ship is deleted?
   -- NOCOM(#sirver): what if destination is deleted while en route?
   -- NOCOM(#sirver): what if the farm disappears? what should happen to the wares then.


   -- NOCOM(#sirver): das ist doch alles bullshit. Wenn ein schiff, oder ein port verschwindet müssen alle anderen ports geupdated werden und die waren müssen eine chance kriegen ihren weg zu canceln.

   port2:remove()
   sleep(20000)
   assert_equal(port1:get_workers("builder"), 1)

   print("# All Tests passed.")
   wl.ui.MapView():close()
end)
