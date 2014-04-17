include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

		--rt(h1(_"The Barbarian Fisher's Hut")) ..
	--Lore Section
	-- TODO crashes
	--building_help_lore_string("barbarians", building_description, _[[‘What do you mean, my fish ain’t fresh?!’]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "blackwood",
		_"Fishes on the coast near the hut.",
		"", "7") ..

	--Dependencies
	rt(h2(_"Dependencies")) ..
	rt(p(_"The Fisher’s Hut needs water full of fish in the vicinity.")) ..

	building_help_outputs("barbarians", building_description, {"battlearena", "trainingcamp", "tavern", "inn", "big_inn"}, true) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"fisher"}, "fishing_rod") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The fisher pauses %s before going to work again.":bformat("18s"))
   end
}
