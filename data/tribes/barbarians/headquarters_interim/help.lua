-- The Barbarian Interim Headquarters

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "carrier",
		_"Accommodation for your people. Also stores your wares and tools.",
		_"The headquarters is your main building." .. "<br>" .. _[[Text needed]]) ..

	--Building Section
	building_help_building_section("barbarians", building_description)
   end
}
