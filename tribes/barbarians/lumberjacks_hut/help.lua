use("aux", "formatting")
use("aux", "format_help")


return {
   func = function(building_description)
      set_textdomain("tribe_barbarians")

      -- TODO(GunChleoc): the building_description is now passed into this
      -- function. I suggest adding methods to get to the image and menu
      -- picture into the description (they are in the c++ class as well). Then, we can get rid of hardcoding everything in here.
		--rt(h1(_"The Barbarian Lumberjack's Hut")) ..
	--Lore Section
		return 
	-- code/testing example for reading building_descr rt(h2("XXX TODO TEST" .. tostring(wl.Game():get_building_description("barbarians","lumberjacks_hut").buildable))) ..
	building_help_lore_string("barbarians", "lumberjacks_hut",_[["Take 200 hits to fell a tree and you're a baby. Take 100 and you're a soldier. Take 50 and you're a hero. Take 20 and soon you will be a honorable lumberjack."]],_[[Krumta, carpenter of Chat'Karuth]]) ..
	--General Section
		building_help_general_string("barbarians", "trunk", 
			_"Fell trees in the surrounding area and process them into trunks.", "10", "--", "4") ..

	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(p(_"The Lumberjack's Hut needs trees in the immediate vicinity to fell.")) ..

		building_help_incoming_list("barbarians",{}) ..

		-- TODO for testing only, this building has no incoming resources
		building_help_incoming_list("barbarians",{
			{"lumberjacks_hut", "trunk"}
		}) ..

		building_help_collecting_list("barbarians",{
			{"lumberjacks_hut", "trunk"}
		}) ..

		building_help_outgoing_list("barbarians",{
			{"constructionsite", "trunk"},
			{"burners_house", "trunk"},
			{"metalworks", "trunk"},
			{"hardener", "trunk"}
		}) ..


	--Building Section
		rt(h2(_"Building")) ..
		building_help_size_string("barbarians", "lumberjacks_hut") ..
		text_line(_"Upgraded from:", "n/a") ..

		rt(h3(_"Build Cost:")) ..

		building_help_cost_list("barbarians", {{"trunk", 3}}) ..

		rt(h3(_"Dismantle yields:")) ..

		building_help_cost_list("barbarians", {{"trunk", 2}}) ..

		text_line(_"Upgradeable to:","n/a") ..

	--Workers Section
		rt(h2(_"Workers")) ..

		rt(h3(_"Crew required:")) ..
		building_help_crew_string("barbarians", "lumberjack") ..

		building_help_tool_string("barbarians", "felling_axe") ..

		text_line(_"Experience levels:","n/a") ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The Lumberjack needs %s to fell a tree, not counting the time he needs to reach the destination and go home again.":format("12s"))
   end
}
