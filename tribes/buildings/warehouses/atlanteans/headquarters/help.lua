-- The Atlantean Headquarters

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_atlanteans")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("atlanteans", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("atlanteans", building_description, "carrier",
		_"Accommodation for your people. Also stores your wares and tools.",
		_"The headquarters is your main building." .. "<br>" .. _[[Text needed]]) ..

	--Building Section
	building_help_building_section("atlanteans", building_description)
   end
}
