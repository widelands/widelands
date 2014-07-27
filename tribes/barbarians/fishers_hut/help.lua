-- The Barbarian Fisher's Hut

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘What do you mean, my fish ain’t fresh?!’]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("barbarians", building_description,
		_"Fishes on the coast near the hut.",
		_"The fisher’s hut needs water full of fish within the working radius.") ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	building_help_production_section(_"The fisher pauses %s before going to work again.":bformat(
		ngettext("%d second", "%d seconds", 18):bformat(18)
	))
   end
}
