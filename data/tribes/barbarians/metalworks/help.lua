-- The Barbarian Metal Workshop

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘We make it work!’]], _[[Inscription on the threshold of the now ruined Olde Forge at Harradsheim, the eldest known smithy.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description,
		_"The barbarian metal workshop can make bread paddles, felling axes, fire tongs, fishing rods, hammers, hunting spears, kitchen tools, picks, scythes and shovels.",
		_"The barbarian metal workshop is the basic production site in a series of three buildings and creates all the tools that barbarians need. The others are for weapons.") ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	building_help_production_section(_"If all needed wares are delivered in time, this building can produce each tool in about %s on average.":bformat(
		ngettext("%d second", "%d seconds", 67):bformat(67)
	))
   end
}
