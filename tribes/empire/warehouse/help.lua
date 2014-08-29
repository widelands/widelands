-- The Imperial Warehouse

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_empire")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("empire", building_description, _[[Text needed]], _[[Source needed]]) ..
	--General Section
	building_help_general_string("empire", building_description,
		_"Warehouses store soldiers, wares and tools.") ..

	--Building Section
	building_help_building_section("empire", building_description)
   end
}
