include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

		--rt(h1(_"The Barbarian Deep Coal Mine")) ..
	--Lore Section
	--text identical to coalmine
	building_help_lore_string("barbarians", building_description, _[[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces. <br>
Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "coal",
		_"Digs coal out of the ground in mountain terrain.",
		_"This mine exploits all of the resource down to the deepest level. But even after having done so, it will still have a %s chance of finding some more coal.":bformat("10%"),
		"2") ..

	--Dependencies
	building_help_inputs("barbarians", building_description, {"big_inn"}, "meal") ..
	building_help_outputs("barbarians", building_description, {"axfactory", "warmill", "helmsmithy", "smelting_works", "lime_kiln"}) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"master-miner", "chief-miner", "miner"}, "pick") ..

	--Building Section
	building_help_building_section("barbarians", building_description, "deep_coalmine") ..

		--rt(h2(_"Building")) ..

-- Upgraded from: OK

		--rt(h3(_"Cost cumulative:")) .. TODO missing
		--help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 6), 6) ..
		--help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 12), 12) ..
		--rt(h3(_"Dismantle yields:")) .. TODO missing
		--help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 3), 3) ..
		--help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 6), 6) ..

	--Production Section
	rt(h2(_"Production")) ..
	text_line(_"Performance:", _"If the food supply is steady, this mine can produce coal in %s on average.":bformat("14.4s"))
  end
}
