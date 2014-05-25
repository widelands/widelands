-- The Barbarian Bakery

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "pittabread",
		_"Bakes pitta bread for soldiers and miners alike.",
		"") ..

	--Dependencies
	building_help_inputs("barbarians", building_description) ..
	building_help_outputs("barbarians", building_description) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"baker"}, "bread_paddle") ..


	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce a pitta bread in %s on average.":bformat("30s"))
  end
}
