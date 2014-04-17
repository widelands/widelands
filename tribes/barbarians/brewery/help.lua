include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

		--rt(h1(_"The Barbarian Brewery")) ..
	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "strongbeer",
		_[[Text needed]],
		_[[Text needed]]) ..

	--Dependencies
	-- TODO

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"master-brewer", "brewer"}) ..

	--Building Section
	building_help_building_section("barbarians", building_description, "micro-brewery", {"micro-brewery"}) ..

	--Production Section
	rt(h2(_"Production")) ..
	text_line(_"Performance:", _[[Text needed]])
  end
}
