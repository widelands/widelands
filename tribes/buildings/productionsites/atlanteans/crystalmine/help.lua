-- The Atlantean Crystal Mine
include "scripting/formatting.lua"
set_textdomain("tribes")
include "tribes/scripting/format_help.lua"

return {
   func = function(building_description)
		return building_help("atlanteans", building_description)
   end
}
