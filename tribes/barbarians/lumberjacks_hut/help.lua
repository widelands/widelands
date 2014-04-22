-- The Barbarian Lumberjack's Hut

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

      -- TODO(GunChleoc): the building_description is now passed into this
      -- function. I suggest adding methods to get to the image and menu
      -- picture into the description (they are in the c++ class as well). Then, we can get rid of hardcoding everything in here.

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[["Take 200 hits to fell a tree and you're a baby. Take 100 and you're a soldier. Take 50 and you're a hero. Take 20 and soon you will be a honorable lumberjack."]],_[[Krumta, carpenter of Chat'Karuth]]) ..
	--General Section
	building_help_general_string("barbarians", building_description, "log",
		_"Fell trees in the surrounding area and process them into logs.",
		_"The lumberjack's hut needs trees to fell within the working radius.", "10") ..

	--Dependencies
	rt(h2(_"Dependencies")) ..

	building_help_outputs("barbarians", building_description, {"constructionsite", "burners_house", "metalworks", "hardener"}, true) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"lumberjack"}, "felling_ax") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	rt(h2(_"Production")) ..
	text_line(_"Performance:", _"The lumberjack needs %s to fell a tree, not counting the time he needs to reach the destination and go home again.":format("12s"))
   end
}
