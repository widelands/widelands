-- The Barbarian Reed Yard

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘We grow roofs’]],_[[Slogan of the Guild of Gardeners]]) ..

	--General Section
	building_help_general_string("barbarians", building_description,
		_"The Reed Yard cultivates reed that serves two different purposes for the Barbarian tribe.",
		_"Thatch reed is the traditional material for roofing, and it is woven into the extremely durable cloth that they use for their ships’ sails.") ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description, true) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	building_help_production_section(_"A reed yard can produce a sheaf of thatch reed in about %s on average.":bformat(
		ngettext("%d second", "%d seconds", 65):bformat(65)
	))
   end
}
