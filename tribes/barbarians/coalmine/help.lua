-- The Barbarian Coal Mine

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces. <br>
Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "coal",
		_"Digs coal out of the ground in mountain terrain.",
		_"This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any coal.":bformat("1/3"), "2") ..

	--Dependencies
	rt(h2(_"Dependencies")) ..

	building_help_inputs("barbarians", building_description, {"tavern", "inn", "big_inn"}, "ration") ..
	building_help_outputs("barbarians", building_description, {"axfactory", "warmill", "helmsmithy", "smelting_works", "lime_kiln"}) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"miner"}, "pick") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	rt(h2(_"Production")) ..
	text_line(_"Performance:", _"If the food supply is steady, this mine can produce coal in %s on average.":bformat("32.5s"))
   end
}
