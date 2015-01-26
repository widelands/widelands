-- The Atlantean Labyrinth
include "scripting/formatting.lua"
set_textdomain("tribes")
include "tribes/scripting/format_help.lua"

return {
   func = function(building_description)
	-- need to get this again, so the building description will be of type "trainingsite"
	local building_description = wl.Game():get_building_description(building_description.name)
	return

	--Lore Section
	building_help_lore_string("atlanteans", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("atlanteans", building_description,
		_"Trains soldiers in ‘Defense’, ‘Evade’, and ‘Health’."
			.. " " .."Equips the soldiers with all necessary weapons and armor parts.") ..

	--Dependencies
	-- We would need to parse the production programs to automate the parameters here; so we do it manually
	dependencies_training("atlanteans", building_description,
		"untrained", "fulltrained-attack",
		"untrained+attack", "fulltrained") ..

	rt(h3(_"Defense Training:")) ..
	dependencies_training_food("atlanteans", { {"smoked_fish", "smoked_meat"}, {"bread_atlanteans"}}) ..
	dependencies_training_weapons("atlanteans", building_description, "and",
		{"shield_steel", "shield_advanced"}, "atlanteans_armorsmithy") ..

	rt(h3(_"Evade Training:")) ..
	dependencies_training_food("atlanteans", { {"smoked_fish", "smoked_meat"}}) ..

	rt(h3(_"Health Training:")) ..
	dependencies_training_food("atlanteans", { {"smoked_fish", "smoked_meat"}, {"bread_atlanteans"}}) ..
	dependencies_training_weapons("atlanteans", building_description, "and",
		{"tabard_golden"}, "atlanteans_weaving_mill") ..

	--Workers Section
	building_help_crew_string("atlanteans", building_description) ..

	--Building Section
	building_help_building_section("atlanteans", building_description) ..

	--Production Section
	building_help_production_section(_[[Calculation needed]])
   end
}
