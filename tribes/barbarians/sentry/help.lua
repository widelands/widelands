include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

		--rt(h1(_"The Barbarian Sentry)) ..
	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[Text needed]], _[[Source needed]]) ..
	--General Section
	building_help_general_string("barbarians", building_description, "soldier", "",
		_"Expands your territory.", "--") ..

	--Building Section
	building_help_building_section("barbarians", building_description)
-- TODO Capacity
   end
}
