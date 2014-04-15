include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return
		--rt(h1(_"The Barbarian Coal Mine")) ..
	--Lore Section

	building_help_lore_string("barbarians", "coalmine",_[[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces. <br>
Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "coal",
		_"A %1$s exploits only %2$s of the resource. From there on out, it will only have a 5%% chance of finding %3$s.":bformat(_"deep coal mine","1/3",_"coal"),
		_"Dig %s out of the ground in mountain terrain.":format(_"coal"),
		"2") ..

	--Dependencies
	rt(h2(_"Dependencies")) ..

	building_help_inputs("barbarians", building_description, {"tavern", "inn", "big_inn"}, "ration") ..
	building_help_outputs("barbarians", building_description, {"axfactory", "warmill", "helmsmithy", "smelting_works", "lime_kiln"}) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"miner"}, "pick") ..

	--Production Section
	rt(h2(_"Production")) ..
	text_line(_"Performance:", _"If the food supply is steady, this mine can produce %s in %s on average.":format(_"Coal","32.5s"))
   end
}
