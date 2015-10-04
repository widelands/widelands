-- The Barbarian Lumberjack's Hut

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘Take 200 hits to fell a tree and you’re a baby. Take 100 and you’re a soldier. Take 50 and you’re a hero. Take 20 and soon you will be a honorable lumberjack.’]],_[[Krumta, carpenter of Chat’Karuth]]) ..
	--General Section
	building_help_general_string("barbarians", building_description,
		_"Fell trees in the surrounding area and process them into logs.",
		_"The lumberjack’s hut needs trees to fell within the work area.") ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description, true) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	building_help_production_section(_"The lumberjack needs %s to fell a tree, not counting the time he needs to reach the destination and go home again.":format(
		ngettext("%d second", "%d seconds", 12):bformat(12)
	))
   end
}
