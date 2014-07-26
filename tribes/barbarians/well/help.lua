-- The Barbarian Well

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘Oh how sweet is the source of life,<br> that comes down from the sky <br> and lets the earth drink.’]], _[[Song written by Sigurd the Bard<br>when the first rain fell after the Great Drought in the 21st year of Chat’Karuth’s reign.]]) ..

	--General Section
	--General Section
	building_help_general_string("barbarians", building_description,
		_"Draws water out of the deep.") ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	building_help_production_section(_"The carrier needs %s to get one bucket full of water.":bformat(
		ngettext("%d second", "%d seconds", 40):bformat(40)
	))
  end
}
