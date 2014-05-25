-- The Barbarian Hunter's Hut

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘As silent as a panther,<br> as deft as a weasel,<br> as swift as an arrow,<br> as deadly as a viper.’]], _[[‘The Art of Hunting’]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "meat",
		_"Hunts animals to produce meat.",
		_"The hunter’s hut needs animals to hunt within the working radius.", "13") ..

	--Dependencies
	building_help_dependencies_production("barbarians", building_description, true) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The hunter pauses %s before going to work again.":bformat("35s"))
   end
}
