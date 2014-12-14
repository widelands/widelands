-- The Atlantean Warehouse
include "scripting/formatting.lua"
set_textdomain("tribes")
include "tribes/scripting/format_help.lua"

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("atlanteans", building_description, _[[Text needed]], _[[Source needed]]) ..
	--General Section
	building_help_general_string("atlanteans", building_description,
		_"Warehouses store soldiers, wares and tools.") ..

	--Building Section
	building_help_building_section("atlanteans", building_description)
   end
}
