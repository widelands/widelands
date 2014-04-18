-- The Barbarian Axfactory

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘A new warrior’s ax brings forth the best in its wielder – or the worst in its maker.’]], _[[An old Barbarian proverb<br> meaning that you need to take some risks sometimes.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "ax",
		_"Produces axes, sharp axes and broad axes.",
		_"The barbarian axfactory is the intermediate production site in a series of three buildings. It is an upgrade from the metal workshop but doesn’t require additional qualification for the worker.") ..

	--Dependencies
	-- TODO where are the resource chain images?
	building_help_inputs("barbarians", building_description, {"smelting_works"}, "iron") ..
	building_help_inputs("barbarians", building_description, {"burners_house", "coalmine"}, "coal") ..

	--building_help_outputs("barbarians", building_description, {"ax", {}, "", {}}) ..

		rt(h3(_"Outgoing:")) ..
-- TODO axfactory-> ax -> headquarters -> soldier
-- TODO axfactory -> broadax -> trainingcamp

		dependencies_basic({"tribes/barbarians/axfactory/menu.png","tribes/barbarians/ax/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/soldier/menu.png"},p(_"Soldier")) ..
		dependencies_basic({"tribes/barbarians/axfactory/menu.png","tribes/barbarians/sharpax/menu.png;tribes/barbarians/broadax/menu.png","tribes/barbarians/trainingcamp/menu.png"},p(_"Training Camp")) ..

	--Workers Section
	building_help_crew_string("barbarians", building_description, {"blacksmith"}, "hammer") ..

	--Building Section
	building_help_building_section("barbarians", building_description, "metalworks", {"metalworks"}) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce each type of ax in about %s on average.":bformat("57s")) ..
		rt(p(_"All three weapons take the same time for making, but the required raw materials vary."))
  end
}
