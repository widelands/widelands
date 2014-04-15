include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

		--rt(h1(_"The Barbarian Deep Coal Mine")) ..
	--Lore Section
	--text identical to coalmine
	building_help_lore_string("barbarians", "deep_coalmine",_[[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces. <br>
Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]]) ..

	--General Section
	-- TODO the last 2 parameters should be reqad from C++. "coal" as well?
	building_help_general_string("barbarians", building_description, "coal",
		_"A %1$s exploits only %2$s of the resource. From there on out, it will only have a 5%% chance of finding %3$s.":bformat(_"deep coal mine","2/3",_"coal"),
		_"Dig %s out of the ground in mountain terrain.":format(_"coal"),
		"2") ..

	--Dependencies
	rt(h2(_"Dependencies")) ..
	rt(h3(_"Incoming:")) ..
	-- TODO read dependencies from C++ and automate
	building_help_dependencies_ware_building("barbarians", {"inn", "snack", "deep_coalmine"}, "snack", "inn") ..
	building_help_dependencies_ware_building("barbarians", {"big_inn", "snack", "deep_coalmine"}, "snack", "big_inn") ..

	rt(h3(_"Outgoing:")) ..
	building_help_dependencies_resi("barbarians", {"resi_coal2", "deep_coalmine", "coal"}, "coal") ..
	building_help_dependencies_building("barbarians",{"coal","axfactory"}, "axfactory") ..
	building_help_dependencies_building("barbarians",{"coal","warmill"}, "warmill") ..
	building_help_dependencies_building("barbarians",{"coal","helmsmithy"}, "helmsmithy") ..
	building_help_dependencies_building("barbarians",{"coal","smelting_works"}, "smelting_works") ..
	building_help_dependencies_building("barbarians",{"coal","lime_kiln"}, "lime_kiln") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	-- TODO read from C++
	--text_line(_"Upgraded from:",_"Coal mine","tribes/barbarians/coalmine/menu.png") ..
	-- add this line at end of building section
	text_line(_"Upgradeable to:",_"Deeper Coal Mine","tribes/barbarians/deeper_coalmine/menu.png") ..

	-- TODO this is empty
	--rt(h3(_"Cost cumulative:")) ..
	--help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 4), 4) ..
	--help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 8), 8) ..
	--rt(h3(_"Dismantle yields:")) ..
	-- TODO: this needs to be cumulative
	--help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
	--help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 4), 4) ..
	-- TODO - already in format_help, but value is missing
	--text_line(_"Upgraded from:",_"Coal mine","tribes/barbarians/coalmine/menu.png") ..

	--Workers Section
	-- TODO this duplicates the tool etc. - get worker list from C++ and handle in format_help
	building_help_crew_string("barbarians", "coalmine", "chief-miner", "pick") ..
	building_help_crew_string("barbarians", "coalmine", "miner", "pick") ..

	--Production Section
	rt(h2(_"Production")) ..
	text_line(_"Performance:", _"If the food supply is steady, this mine can produce %1$s in %2$s on average.":bformat(_"Coal","19.5s"))
   end
}
