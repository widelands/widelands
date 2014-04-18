-- The Barbarian Deep Gold Mine

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	--text identical to goldmine
	building_help_lore_string("barbarians", building_description, _[[‘Soft and supple.<br> And yet untouched by time and weather.<br> Rays of sun, wrought into eternity ...’]], _[[Excerpt from ‘Our Treasures Underground’,<br> a traditional Barbarian song.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "goldore",
		_"Digs gold ore out of the ground in mountain terrain.",
		_"This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any gold ore.":bformat("2/3"),
		"2") ..

	--Dependencies
	building_help_inputs("barbarians", building_description, {"inn", "big_inn"}, "snack") ..
-- TODO the following line causes a crash
--	building_help_outputs("barbarians", building_description, {"smelting_works"}) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"chief-miner", "miner"}, "pick") ..

	--Building Section
	building_help_building_section("barbarians", building_description, "goldmine", {"goldmine"}) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If the food supply is steady, this mine can produce gold ore in %s on average.":bformat("19.5s"))
  end
}
