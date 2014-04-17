include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

		--rt(h1(_"The Barbarian Gamekeeper's Hut")) ..
	--Lore Section
--	building_help_lore_string("barbarians", building_description, _[[‘He loves the animals and to breed them<br>
--as we love to cook and eat them.’]],_[[Barbarian nursery rhyme]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "meat",
		_"Releases animals into the wild to steady the meat production.",
		"",
		"3") ..

	--Dependencies: none

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"gamekeeper"}) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The gamekeeper pauses %s before going to work again.":bformat("52.5s"))
   end
}
