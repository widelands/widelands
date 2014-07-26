-- The Barbarian Gamekeeper's Hut

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description,
		_[[‘He loves the animals and to breed them<br>
			as we love to cook and eat them.’]],
		_[[Barbarian nursery rhyme]]) ..

	--General Section
	building_help_general_string("barbarians", building_description,
		_"Releases animals into the wild to steady the meat production.") ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	building_help_production_section(_"The gamekeeper pauses %s before going to work again.":bformat(
		ngettext("%d second", "%d seconds", 52.5):bformat(52.5)
	))
   end
}
