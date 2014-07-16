-- The Atlantean Port

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_atlanteans")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("atlanteans", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	-- TODO this building acts like a warehouse
	building_help_general_string("atlanteans", building_description, "ship",
		_[[Text needed]]) ..

	--Dependencies
	-- TODO expedition costs here?

	--Building Section
	building_help_building_section("atlanteans", building_description) ..

	--Production Section
	building_help_production_section(_[[Calculation needed]])
   end
}
