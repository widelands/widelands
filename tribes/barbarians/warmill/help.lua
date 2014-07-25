-- The Barbarian War Mill

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
		_"The war mill produces all the axes that the barbarians use for warfare.",
		_"The barbarian war mill is their most advanced production site for weapons. As such it needs to be upgraded from an axfactory.") ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description, "axfactory", {"metalworks", "axfactory"}) ..

	--Production Section
	building_help_production_section(_"If all needed wares are delivered in time, this building can produce each ax in about %s on average.":bformat(
		ngettext("%d second", "%d seconds", 57):bformat(57)
		) .. " " ..
		_"All weapons require the same time for making, but the consumption of the same raw materials increases from step to step. The last two need gold."
	)
   end
}
