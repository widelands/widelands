-- The Barbarian Brewery
include "scripting/formatting.lua"
set_textdomain("tribes")
include "tribes/scripting/format_help.lua"

return {
   func = function(building_description)
	return

		--General Section
		building_help_general_string(building_description) ..

		--Dependencies
		building_help_dependencies_production("barbarians", building_description) ..

		--Workers Section
		building_help_crew_string(building_description) ..

		--Building Section
		building_help_building_section(building_description, "barbarians_micro_brewery", {"barbarians_micro_brewery"}) ..

		--Production Section
		building_help_production_section()
	end
}
