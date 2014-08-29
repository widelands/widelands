-- The Barbarian Deep Coal Mine

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	--text identical to coalmine
	building_help_lore_string("barbarians", building_description, _[[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces. <br>
Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]]) ..

	--General Section
	-- Keeping the parameters here hardcoded for the moment, too hard to get from C++.
	building_help_general_string("barbarians", building_description,
		_"Digs coal out of the ground in mountain terrain.",
		_"This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any coal.":bformat("2/3")) ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description, "coalmine", {"coalmine"}) ..

	--Production Section
	building_help_production_section(_"If the food supply is steady, this mine can produce coal in %s on average.":bformat(
		ngettext("%d second", "%d seconds", 19.5):bformat(19.5)
	))
   end
}
