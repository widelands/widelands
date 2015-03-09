run(function()
   -- NOCOM
    game.desired_speed = 15 * 1000
	p1:place_bob("ship", map:get_field(10, 10))

	port = map:get_field(16, 16).immovable
 	port:set_wares("log", 10) -- no sense to wait
	port:set_wares("blackwood", 10) 
	 
   	ships = p1:get_ships()
 	
   	--should contain 1 item (1 ship)
  	assert_equal(1, #ships)

  	--ship has no wares on it
  	--(at the same time testing that that array really contains a ship)
  	assert_equal(0,ships[1]:get_wares())
  	
  	--no destination is set
  	assert(not ships[1].destination)
 
  	--ships in transport state (=0)
  	assert_equal(0,ships[1].status)

	--starting prepartion for expedition
   	port:start_expedition()	
   	
	--ships changes status when exp ready
   	while ships[1].status == 0 do sleep(2000) end

 	--sending NW and verifying
   	ships[1].scout_direction=6
   	sleep(3000)
  	assert_equal(6, ships[1].scout_direction)
  	
   	while ships[1].scout_direction == 6 do
   		sleep (2000)
   	end

	--now ships stops nearby coast, sending it back
	ships[1].scout_direction=3
	sleep(3000)
	assert_equal(3, ships[1].scout_direction)
	
	--waiting till it stops (value=255)
	while ships[1].scout_direction <255 do sleep(2000) end
	
	--sending scouting the island
	ships[1].island_scout_direction=1;
	sleep(3000)	
	assert_equal(1, ships[1].island_scout_direction)
	
	--fine, now change the direction
	ships[1].island_scout_direction=0;
	sleep(3000)	
	assert_equal(0, ships[1].island_scout_direction)

    -- wait till it finds a port
    wait_for_message("Port Space Found")	
	--starting colonization port here
	assert(ships[1]:build_colonization_port())
	sleep(15000)
	stable_save("port_in_constr")

	-- while unfinished yet, removing it
	new_port=map:get_field(16,2).immovable
	assert(new_port)
	new_port:remove()
	sleep(3000)
	
	--yes, the ships is back in transport mode
	assert(0,ships[1].status)
	
	print("# All Tests passed.")
   	wl.ui.MapView():close()

end)
