include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

		--rt(h1(_"The Barbarian Quarry")) ..
	--Lore Section
	-- TODO crashes
--	building_help_lore_string("barbarians", building_description, _[[Text needed]],_[[Source needed]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "raw_stone",
		_"Carves raw stone out of rocks lying near the hut.", "", "6") ..

	--Dependencies
		rt(h2(_"Dependencies")) ..
	rt(p(_"The quarry needs stones in the vicinity.")) ..

	building_help_outputs("barbarians", building_description, {"constructionsite", "lime_kiln"}, true) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"stonemason"}, "pick") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The stonemason pauses %s before going back to work again.":bformat("65s"))
   end
}
