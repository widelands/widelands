include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

		--rt(h1(_"The Barbarian Farm")) ..
	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "wheat",
		_"Sow and harvest wheat.",
		"",
		"2") ..

	--Dependencies
	building_help_outputs("barbarians", building_description, {"bakery", "micro-brewery", "brewery", "cattlefarm"}) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"farmer"}, "scythe") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"Calculation needed")
   end
}
