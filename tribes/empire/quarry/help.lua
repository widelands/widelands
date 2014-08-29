-- The Imperial Quarry

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_empire")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("empire", building_description, _[[Text needed]],_[[Source needed]]) ..

	--General Section
	building_help_general_string("empire", building_description,
		_"Carves marble and stone out of rocks in the vicinity.", _"The quarry needs stones to cut within the working radius.") ..

	--Dependencies
	building_help_dependencies_production("empire", building_description, true) ..

	--Workers Section
	building_help_crew_string("empire", building_description) ..

	--Building Section
	building_help_building_section("empire", building_description) ..

	--Production Section
	building_help_production_section(_[[Calculation needed]])
   end
}
