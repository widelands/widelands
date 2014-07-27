-- The Atlantean Dungeon

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_atlanteans")

return {
   func = function(building_description)
	-- need to get this again, so the building description will be of type "trainingsite"
	local building_description = wl.Game():get_building_description("atlanteans", building_description.name)
	return

	--Lore Section
	building_help_lore_string("atlanteans", building_description, _[[Text needed]], _[[Source needed]]) ..

	--General Section
	building_help_general_string("atlanteans", building_description,
		_"Trains soldiers in ‘Attack’."
			.. " " .."Equips the soldiers with all necessary weapons and armor parts.") ..

	--Dependencies
	-- We would need to parse the production programs to automate the parameters here; so we do it manually
	dependencies_training("atlanteans", building_description,
		"untrained", "untrained+attack",
		"fulltrained-attack", "fulltrained") ..

	rt(h3(_"Attack Training:")) ..
	dependencies_training_food("atlanteans", { {"smoked_fish", "smoked_meat"}, {"bread"}}) ..
	dependencies_training_weapons("atlanteans", building_description, "and",
		{"long_trident", "steel_trident", "double_trident", "heavy_double_trident"}, "weaponsmithy") ..

	--Workers Section
	building_help_crew_string("atlanteans", building_description) ..

	--Building Section
	building_help_building_section("atlanteans", building_description) ..

	--Production Section
	building_help_production_section(_[[Calculation needed]])
   end
}
