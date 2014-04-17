include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return
		--rt(h1(_"The Barbarian Hunter's Hut")) ..
	--Lore Section
-- TODO crashes
--	building_help_lore_string("barbarians", building_description, _[[‘As silent as a panther,<br> as deft as a weasel,<br> as swift as an arrow,<br> as deadly as a viper.’]], _[[‘The Art of Hunting’]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "meat",
		_"Hunts animals to produce meat.",
		"", "13") ..

	--Dependencies
	rt(h2(_"Dependencies")) ..
	rt(p(_"The Hunter’s Hut needs animals in the vicinity to hunt.")) ..

	building_help_outputs("barbarians", building_description, {"battlearena", "trainingcamp", "tavern", "inn", "big_inn"}, true) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"hunter"}, "hunting_spear") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The hunter pauses %s before going to work again.":bformat("35s"))
   end
}
