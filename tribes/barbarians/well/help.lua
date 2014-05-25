-- The Barbarian Well

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘Oh how sweet is the source of life,<br> that comes down from the sky <br> and lets the earth drink.’]], _[[Song written by Sigurd the Bard<br>when the first rain fell after the Great Drought in the 21st year of Chat’Karuth’s reign.]]) ..

	--General Section
	--General Section
	building_help_general_string("barbarians", building_description, "coal",
		_"Draws water out of the deep.",
		"", "1") ..

	--Dependencies
	rt(h2(_"Dependencies")) ..
	building_help_outputs("barbarians", building_description, true) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"carrier"}) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The carrier needs %s to get one bucket full of water.":format("40s"))
  end
}
