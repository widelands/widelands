include "scripting/formatting.lua"
include "scripting/format_help.lua"


return {
   func = function(building_description)
      set_textdomain("tribe_barbarians")

      -- TODO(GunChleoc): the building_description is now passed into this
      -- function. I suggest adding methods to get to the image and menu
      -- picture into the description (they are in the c++ class as well). Then, we can get rid of hardcoding everything in here.
		--rt(h1(_"The Barbarian Lumberjack's Hut")) ..
	--Lore Section
		return

	building_help_lore_string("barbarians", "lumberjacks_hut",_[["Take 200 hits to fell a tree and you're a baby. Take 100 and you're a soldier. Take 50 and you're a hero. Take 20 and soon you will be a honorable lumberjack."]],_[[Krumta, carpenter of Chat'Karuth]]) ..
	--General Section
	building_help_general_string("barbarians", building_description, "log", "",
		_"Fell trees in the surrounding area and process them into logs.", "10", "--") ..

	--Dependencies
	rt(h2(_"Dependencies")) ..
	rt(p(_"The Lumberjack's Hut needs trees in the immediate vicinity to fell.")) ..

	rt(h3(_"Incoming:")) ..
	-- TODO remove these, just for testing
	--building_help_depencencies_ware("barbarians",{"inn","snack","deep_coalmine"}, "snack") ..
	--building_help_depencencies_building("barbarians",{"coal","axfactory"}, "axfactory") ..

	rt(h3(_"Collects:")) ..
	--building_help_depencencies_ware("barbarians", {"lumberjacks_hut", "log"}, "log") ..


	rt(h3(_"Outgoing:")) ..
	building_help_depencencies_ware("barbarians", {"constructionsite", "log"}, "log") ..
	--building_help_depencencies_ware("barbarians", {"burners_house", "log"}, "log") ..
	--building_help_depencencies_ware("barbarians", {"metalworks", "log"}, "log") ..
	--building_help_depencencies_ware("barbarians", {"hardener", "log"}, "log") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Workers Section
	rt(h2(_"Workers")) ..

	rt(h3(_"Crew required:")) ..
	building_help_crew_string("barbarians", "lumberjack") ..

	building_help_tool_string("barbarians", "felling_ax") ..

	text_line(_"Experience levels:","n/a") ..

	--Production Section
	rt(h2(_"Production")) ..
	text_line(_"Performance:", _"The Lumberjack needs %s to fell a tree, not counting the time he needs to reach the destination and go home again.":format("12s"))
   end
}
