include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

		--rt(h1(_"The Barbarian Wood Hardener")) ..
	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "blackwood",
		_[[Text needed]],
		"") ..

	--Dependencies
	building_help_inputs("barbarians", building_description, {"lumberjacks_hut"}, "log") ..
	-- TODO outputs

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"lumberjack"}, "felling_ax") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"Calculation needed")
   end
}
