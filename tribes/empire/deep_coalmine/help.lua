-- The Imperial Deep Coal Mine

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_empire")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("empire", building_description, _[[Text needed]]) ..

	--General Section
	-- Keeping the parameters here hardcoded for the moment, too hard to get from C++.
	building_help_general_string("empire", building_description,
		_"Digs coal out of the ground in mountain terrain.") ..

	--Dependencies
	building_help_dependencies_production("empire", building_description) ..

	--Workers Section
	building_help_crew_string("empire", building_description) ..

	--Building Section
	building_help_building_section("empire", building_description, "coalmine", {"coalmine"}) ..

	--Production Section
	building_help_production_section(_[[Calculation needed]])
   end
}
