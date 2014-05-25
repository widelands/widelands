-- The Barbarian Farm

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	-- TODO this building acts like a warehouse
	building_help_general_string("barbarians", building_description, "ship",
		_[[Text needed]],
		"") ..

	--Dependencies
	-- TODO

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"Calculation needed")
   end
}
