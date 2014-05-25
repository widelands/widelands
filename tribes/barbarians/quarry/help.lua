-- The Barbarian Quarry

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[Text needed]],_[[Source needed]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "raw_stone",
		_"Carves raw stone out of rocks lying near the hut.", _"The quarry needs stones to cut within the working radius.", "6") ..

	--Dependencies
		rt(h2(_"Dependencies")) ..

	building_help_outputs("barbarians", building_description, true, true) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The stonemason pauses %s before going back to work again.":bformat("65s"))
   end
}
