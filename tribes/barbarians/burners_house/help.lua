include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return
		--rt(h1(_"The Barbarian Charcoal Burner's House")) ..
	--Lore Section
	-- TODO this crashes: Could not find filename: could not find file or directory: tribes/barbarians/burners_house/burners_house_i_00.png
	--building_help_lore_string("barbarians", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "coal",
		_"Bakes pitta bread for soldiers and miners alike.",
		"") ..

	--Dependencies
	-- TODO

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"burner"}) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _[[Text needed]])
  end
}
