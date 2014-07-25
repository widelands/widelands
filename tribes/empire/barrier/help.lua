-- The Imperial Barrier

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
		_"Garrisons soldiers to expand your territory.",
		_"If you’re low on soldiers to occupy new military sites, use the downward arrow button to decrease the capacity. You can also click on a soldier to send him away.") ..

	--Building Section
	-- This building can both be upgraded or built directly.
	-- Display build cost, upgrade cost, and dismantle costs for both versions.
	-- Dismantle returns for upgraded version: 2 stone, 1 wood, 1 marble
	-- Dismantle returns for built version: 1 stone, 1 log, 1 wood
	building_help_building_section("empire", building_description, "outpost", {"outpost"})
   end
}
