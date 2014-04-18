-- The Barbarian Smelting Works

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
-- TODO crashes
	building_help_lore_string("barbarians", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "iron",
		_"Smelts iron ore into iron and gold ore into gold.", "") ..

	--Dependencies
	-- TODO

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"smelter"}, "fire_tongs") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _[[Text needed]])
   end
}
