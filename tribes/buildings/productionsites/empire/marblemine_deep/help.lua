-- The Imperial Deep Marble Mine
include "scripting/formatting.lua"
set_textdomain("tribes")
include "tribes/scripting/format_help.lua"

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("empire", building_description, _[[Text needed]], _[[Text needed]]) ..

	--General Section
	building_help_general_string("empire", building_description,
		_"Carves marble and granite out of the rock in mountain terrain.") ..

	--Dependencies
	building_help_dependencies_production("empire", building_description) ..

	--Workers Section
	building_help_crew_string("empire", building_description) ..

	--Building Section
	building_help_building_section("empire", building_description, "empire_marblemine", {"empire_marblemine"}) ..

	--Production Section
	building_help_production_section(_[[Calculation needed]])
  end
}
