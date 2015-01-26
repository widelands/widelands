-- The Imperial Barrier
include "scripting/formatting.lua"
set_textdomain("tribes")
include "tribes/scripting/format_help.lua"

return {
   func = function(building_description)
	return

	--General Section
	building_help_general_string(building_description) ..

	--Building Section
	-- This building can both be upgraded or built directly.
	-- Display build cost, upgrade cost, and dismantle costs for both versions.
	-- Dismantle returns for upgraded version: 2 granite, 1 planks, 1 marble
	-- Dismantle returns for built version: 1 granite, 1 log, 1 planks
	building_help_building_section(building_description, "empire_outpost", {"empire_outpost"})
   end
}
