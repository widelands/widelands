-- The Imperial Headquarters Shipwreck

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_empire")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("empire", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("empire", building_description, "carrier",
		_"Although this ship ran aground, it still serves as accommodation for your people. It also stores your wares and tools.",
		_"The headquarters shipwreck is your main building." .. "<br>" .. _[[Text needed]]) ..

	--Building Section
	building_help_building_section("empire", building_description)
   end
}
