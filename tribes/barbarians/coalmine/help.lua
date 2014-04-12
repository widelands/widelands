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
		_"A %s exploits only %s of the resource. <br> From there on, it will only have a 5%% chance of finding %s.":format(_"coal mine","1/3",_"coal"),
		_"Dig %s out of the ground in mountain terrain.":format(_"coal"),
		"2", "--") ..
		--text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..

		building_help_dependencies_ware_building("barbarians", {"tavern", "ration", "coalmine"}, "ration", "tavern") ..
		building_help_dependencies_ware_building("barbarians", {"inn", "ration", "coalmine"}, "ration", "inn") ..
		building_help_dependencies_ware_building("barbarians", {"big_inn", "ration", "coalmine"}, "ration", "big_inn") ..

		rt(h3(_"Outgoing:")) ..
		building_help_dependencies_resi("barbarians", {"resi_coal2", "coalmine", "coal"}, "coal") ..
		building_help_dependencies_building("barbarians",{"coal","axfactory"}, "axfactory") ..
		building_help_dependencies_building("barbarians",{"coal","warmill"}, "warmill") ..
		building_help_dependencies_building("barbarians",{"coal","helmsmithy"}, "helmsmithy") ..
		building_help_dependencies_building("barbarians",{"coal","smelting_works"}, "smelting_works") ..
		building_help_dependencies_building("barbarians",{"coal","lime_kiln"}, "lime_kiln") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..
		rt(h2(_"Building")) ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build Cost:")) ..
		rt(h3(_"Dismantle yields:")) ..
		text_line(_"Upgradeable to:",_"Deep Coal Mine","tribes/barbarians/deep_coalmine/menu.png") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		building_help_crew_string("barbarians", "coalmine", "miner", "pick") ..

		rt(h3(_"Experience levels:")) ..
		rt("text-align=right", p(_"%s to %s (%s EP)":format(_"Miner",_"Chief Miner","19") .. "<br>" .. _"%s to %s (%s EP)":format(_"Chief Miner",_"Master Miner","28"))) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If the food supply is steady, this mine can produce %s in %s on average.":format(_"Coal","32.5s"))
   end
}
