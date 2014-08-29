-- The Barbarian Lime Kiln

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘Forming new stone from old with fire and water.’]], _[[Ragnald the Child’s answer to the question, what he’s doing in the dirt. His ‘new stone’ now is an important building material.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description,
		_[[The Lime Kiln processes Raw Stones to make ‘Grout’, a substance that solidifies and so reinforces masonry.]],
		_"The Lime Kiln’s output will only go to construction sites that need it. Those are predominantly houses that work with fire, and some military sites.") ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description, true) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	building_help_production_section(_"If all needed wares are delivered in time, this building can produce grout in about %s on average.":bformat(
		ngettext("%d second", "%d seconds", 41):bformat(41)
	))
   end
}
