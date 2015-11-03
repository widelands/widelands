run(function()
   game.desired_speed = 30 * 1000

    -- placing a ship on coast
	p1:place_ship(map:get_field(8, 8))
	sleep(1000)

	--getting table with all our ships (single one only)
	ships = p1:get_ships()

	--ships table should contain 1 item (1 ship)
  	assert_equal(1, #ships)

	--waiting till it is pulled from coast
	while ships[1].field.x == 8 and ships[1].field.xy == 8 do
		print ("ship still on coast")
		sleep(1000)
	end

	port = map:get_field(16, 16).immovable
 	port:set_wares("log", 10) -- no sense to wait
	port:set_wares("blackwood", 10)

  	--ship has no wares on it
  	assert_equal(0,ships[1]:get_wares())

  	--no destination is set
  	assert(not ships[1].destination)
  	--ships in transport state
  	assert_equal("transport", ships[1].state)

	--the  warehouse is probably not in expedition state :)
	assert(not map:get_field(8, 18).immovable.expedition_in_progress)

	--starting prepartion for expedition
	assert(not port.expedition_in_progress)
	port:start_expedition()
	sleep (300)
	assert(port.expedition_in_progress)
	assert(ships[1])

	--ships changes state when exp ready
	while ships[1].state == "transport" do sleep(2000) end
	assert_equal("exp_waiting", ships[1].state)
 	--sending NW and verifying
	ships[1].scouting_direction="nw"

	sleep(6000)
  	assert_equal("nw", ships[1].scouting_direction)

  	assert_equal("exp_scouting", ships[1].state)

	while ships[1].scouting_direction == "nw" do
		sleep (2000)
	end

	--now ships stops nearby NW coast, so sending it back
	ships[1].scouting_direction="se"
	sleep(4000)
	assert_equal("se", ships[1].scouting_direction)

	--testing remaining directions
	ships[1].scouting_direction="e"
	sleep(2000)
	assert_equal("e", ships[1].scouting_direction)

	ships[1].scouting_direction="w"
	sleep(2000)
	assert_equal("w", ships[1].scouting_direction)

	ships[1].scouting_direction="sw"
	sleep(2000)
	assert_equal("sw", ships[1].scouting_direction)

	ships[1].scouting_direction="ne"
	sleep(2000)
	assert_equal("ne", ships[1].scouting_direction)

	--back to original course
	ships[1].scouting_direction="se"
	sleep(2000)
	assert_equal("se", ships[1].scouting_direction)

	--waiting till it stops (no direction/nil is returned)
	while ships[1].scouting_direction do sleep(2000) end

	--sending to scout the island
	ships[1].island_explore_direction="ccw";
	sleep(3000)
	assert_equal("ccw", ships[1].island_explore_direction)
	assert_equal("exp_scouting", ships[1].state)

	--fine, now change the direction
	ships[1].island_explore_direction="cw";
	sleep(3000)
	assert_equal("cw", ships[1].island_explore_direction)

	-- wait till it finds a port
	wait_for_message("Port Space Found")
	sleep(500)
	assert_equal("exp_found_port_space", ships[1].state)

	--starting colonization port here
	assert(ships[1]:build_colonization_port())
	sleep(500)
	assert_equal("exp_colonizing", ships[1].state)
	sleep(15000)
	stable_save("port_in_constr")

	-- while unfinished yet, removing it
	new_port=map:get_field(16,2).immovable
	assert(new_port)
	new_port:remove()
	sleep(3000)

	--yes, the ships is back in transport mode
	assert_equal("transport", ships[1].state)

	print("# All Tests passed.")
	wl.ui.MapView():close()
end)
