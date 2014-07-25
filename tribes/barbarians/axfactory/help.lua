-- The Barbarian Axfactory

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘A new warrior’s ax brings forth the best in its wielder – or the worst in its maker.’]], _[[An old Barbarian proverb<br> meaning that you need to take some risks sometimes.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description,
		_"Produces axes, sharp axes and broad axes.",
		_"The barbarian axfactory is the intermediate production site in a series of three buildings. It is an upgrade from the metal workshop but doesn’t require additional qualification for the worker.") ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description, "metalworks", {"metalworks"}) ..

	--Production Section
	building_help_production_section(_"If all needed wares are delivered in time, this building can produce each type of ax in about %s on average.":bformat(
		ngettext("%d second", "%d seconds", 57):bformat(57)
		) .. " " ..
		_"All three weapons take the same time for making, but the required raw materials vary."
	)
  end
}
