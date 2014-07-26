-- The Barbarian Deeper Iron Ore Mine

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	--text identical to oremine
	building_help_lore_string("barbarians", building_description, _[[‘I look at my own pick wearing away day by day and I realize why my work is important.’]], _[[Quote from an anonymous miner.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description,
		_"Digs iron ore out of the ground in mountain terrain.",
		_"This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more iron ore.":bformat("10%")) ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description, "deep_oremine", {"oremine", "deep_oremine"}) ..

	--Production Section
	building_help_production_section(_"If the food supply is steady, this mine can produce iron ore in %s on average.":bformat(
		ngettext("%d second", "%d seconds", 17.6):bformat(17.6)
	))
  end
}
