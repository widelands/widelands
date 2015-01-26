-- The Imperial Sentry
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
	-- Dismantle returns the same for both versions
	building_help_building_section(building_description, "empire_blockhouse", {"empire_blockhouse"})
   end
}
