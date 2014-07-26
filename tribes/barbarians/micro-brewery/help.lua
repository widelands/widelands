-- The Barbarian Micro Brewery

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘Let the first one drive away the hunger, the second one put you at ease; when you have swallowed up your third one, it’s time for the next shift!’]], _[[Widespread toast among Miners]]) ..

	--General Section
	building_help_general_string("barbarians", building_description,
		_"The micro brewery produces Beer of the lower grade. This beer is a vital component of the snacks that inns and big inns prepare for miners in deep mines.") ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	building_help_production_section(_"If all needed wares are delivered in time, this building can produce beer in about %s on average.":bformat(
		ngettext("%d second", "%d seconds", 60):bformat(60)
	))
  end
}
