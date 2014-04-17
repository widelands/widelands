include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return
		--rt(h1(_"The Barbarian Lime Kiln")) ..
	--Lore Section
-- TODO crashes
--	building_help_lore_string("barbarians", building_description, _[[‘Forming new stone from old with fire and water.’]], _[[Ragnald the Child’s answer to the question, what he’s doing in the dirt. His ‘new stone’ now is an important building material.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "grout",
		_[[The Lime Kiln processes Raw Stones to make ‘Grout’, a substance that solidifies and so reinforces masonry.]],
		_"The Lime Kiln’s output will only go to construction sites that need it. Those are predominantly houses that work with fire, and some military sites.") ..

	--Dependencies
		rt(h2(_"Dependencies")) ..

	building_help_inputs("barbarians", building_description, {"quarry", "granitemine"}, "raw_stone") ..
	building_help_inputs("barbarians", building_description, {"coalmine", "burners_house"}, "coal") ..
	building_help_inputs("barbarians", building_description, {"well"}, "water") ..

	building_help_outputs("barbarians", building_description, {"constructionsite", "lime_kiln"}) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"lime-burner"}) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce grout in about %s on average.":bformat("41s"))
   end
}
