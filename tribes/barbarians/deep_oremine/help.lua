-- The Barbarian Deep Iron Ore Mine

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	--text identical to oremine
	building_help_lore_string("barbarians", building_description, _[[‘I look at my own pick wearing away day by day and I realize why my work is important.’]], _[[Quote from an anonymous miner.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "ironore",
		_"Digs iron ore out of the ground in mountain terrain.",
		_"This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any iron ore.":bformat("2/3"),
		"2") ..

	--Dependencies
	building_help_inputs("barbarians", building_description, {"inn", "big_inn"}, "snack") ..
-- TODO the following line causes a crash
--	building_help_outputs("barbarians", building_description, {"smelting_works"}) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"chief-miner", "miner"}, "pick") ..

	--Building Section
	building_help_building_section("barbarians", building_description, "oremine", {"oremine"}) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If the food supply is steady, this mine can produce iron ore in %s on average.":bformat("39.5s"))
  end
}
