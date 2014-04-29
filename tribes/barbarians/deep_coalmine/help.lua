-- The Barbarian Deep Coal Mine

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	--text identical to coalmine
	building_help_lore_string("barbarians", building_description, _[[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces. <br>
Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]]) ..

	--General Section
	-- TODO the last parameter (Working radius) should be read from C++. "coal" as well?
   -- NOCOM(#gunchleoc): yes, both. But this is a harder as then we would need to parse and understand the programs - which can be arbitrarily complicated. Not sure how to handle that right now - so let's hardcode for the moment.
	building_help_general_string("barbarians", building_description, "coal",
		_"Digs coal out of the ground in mountain terrain.",
		_"This mine exploits only %s of the resource. From there on out, it will only have a 5%% chance of finding any coal.":bformat("2/3"),
		"2") ..

	--Dependencies
	rt(h2(_"Dependencies")) ..

	-- TODO read dependencies from C++ and automate
   -- -- NOCOM(#gunchleoc): not sure what you want to display here.
	building_help_inputs("barbarians", building_description, {"inn", "big_inn"}, "snack") ..
	building_help_outputs("barbarians", building_description, {"axfactory", "warmill", "helmsmithy", "smelting_works", "lime_kiln"}) ..

	--Workers Section
	-- TODO get worker list from C++ and handle in format_help
	-- TODO get tool from C++
   -- -- NOCOM(#gunchleoc): needs more wrapping - especially for the worker description.
	building_help_crew_string("barbarians", building_description, {"chief-miner", "miner"}, "pick") ..

	--Building Section
	building_help_building_section("barbarians", building_description, "coalmine", {"coalmine"}) ..

	--Production Section
	rt(h2(_"Production")) ..
	text_line(_"Performance:", _"If the food supply is steady, this mine can produce coal in %s on average.":bformat("19.5s"))
   end
}
