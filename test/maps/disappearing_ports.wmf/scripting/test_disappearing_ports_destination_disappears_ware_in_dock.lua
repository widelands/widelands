run(function()
   sleep(100)
   game.desired_speed = 10 * 1000

   create_first_port()
   create_second_port()

   game.desired_speed = 1 * 1000
   sleep(1000)
   start_building_farm()
   port1:set_wares{
      blackwood = 1,
   }
   flag = port1.flag

   run(function()
      while true do
         sleep(1000)
         print("flag: ", flag.debug_economy)
         print("ship: ", ship.debug_economy)
         if port1 then
            print("port1: ", port1.portdock.debug_economy)
         end
         if port2 then
            print("port2: ", port2.portdock.debug_economy)
         end
      end
   end)
   -- sleep(10000)

   -- NOCOM(#sirver): before the ship grabs the ware
   -- NOCOM(#sirver): after the ship has the ware.
   -- NOCOM(#sirver): also saving and loading
   -- port2:remove()
   -- sleep(20000)
   -- assert_equal(port1:get_wares("blackwood"), 1)

   -- print("# All Tests passed.")
   -- wl.ui.MapView():close()
end)
