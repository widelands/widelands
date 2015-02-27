-- The Barbarian Training Camp

include "scripting/formatting.lua"
include "tribes/scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	-- need to get this again, so the building description will be of type "trainingsite"
	local building_description = wl.Game():get_building_description("barbarians", building_description.name)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘He who is strong shall neither forgive nor forget, but revenge injustice suffered – in the past and for all future.’]], _[[Chief Chat’Karuth in a speech to his army.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description,
		_"Trains soldiers in ‘Attack’ and in ‘Health’."
			.. " " .."Equips the soldiers with all necessary weapons and armor parts.",
		_"Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level.") ..

	--Dependencies
	-- We would need to parse the production programs to automate the parameters here; so we do it manually
	dependencies_training("barbarians", building_description,
		"untrained", "fulltrained-evade",
		"untrained+evade", "fulltrained") ..

	rt(h3(_"Attack Training:")) ..
	dependencies_training_food("barbarians", { {"fish", "meat"}, {"pittabread"}}) ..
	dependencies_training_weapons("barbarians", building_description, "and", {"sharpax", "broadax"}, "axfactory") ..
	dependencies_training_weapons("barbarians", building_description, "or",
		{"sharpax", "broadax", "bronzeax", "battleax", "warriorsax"}, "warmill") ..

	rt(h3(_"Health Training:")) ..
	dependencies_training_food("barbarians", { {"fish", "meat"}, {"pittabread"}}) ..
	dependencies_training_weapons("barbarians", building_description, "and", {"helm", "mask", "warhelm"}, "helmsmithy") ..

	--Workers Section
	building_help_crew_string("barbarians", building_description) ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
	building_help_production_section(_"If all needed wares are delivered in time, a training camp can train one new soldier in attack and health to the final level in %1$s and %2$s on average.":bformat(
		ngettext("%d minute", "%d minutes", 4):bformat(4),
		ngettext("%d second", "%d seconds", 40):bformat(40)
	))
   end
}
