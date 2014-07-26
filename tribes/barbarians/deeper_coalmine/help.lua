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
	building_help_general_string("barbarians", building_description,
		_"Digs coal out of the ground in mountain terrain.",
		_"This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more coal.":bformat("10%")) ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description, "deep_coalmine", {"coalmine", "deep_coalmine"}) ..

	--Production Section
	building_help_production_section(_"If the food supply is steady, this mine can produce coal in %s on average.":bformat(
		ngettext("%d second", "%d seconds", 14.4):bformat(14.4)
	))
  end
}
