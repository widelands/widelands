-- The Atlantean High Tower
include "scripting/formatting.lua"
set_textdomain("tribes")
include "tribes/scripting/format_help.lua"

return {
   func = function(building_description)
	return

	--General Section
	building_help_general_string(building_description) ..

	--Building Section
	building_help_building_section(building_description, "atlanteans_tower", {"atlanteans_tower"})
   end
}
