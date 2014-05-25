-- The Barbarian War Mill

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
		_"The war mill produces all the axes that the barbarians use for warfare.",
		_"The barbarian war mill is their most advanced production site for weapons. As such it needs to be upgraded from an axfactory.") ..

	--Dependencies
		rt(h2(_"Dependencies")) ..
	building_help_inputs("barbarians", building_description) ..

		rt(h3(_"Outgoing:")) ..
-- TODO warmill-> ax -> headquarters -> soldier
-- TODO warmill -> sharpax, broadax, bronzeax, battleax, warriorsax -> trainingcamp

		--dependencies({"tribes/barbarians/warmill/menu.png","tribes/barbarians/ax/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/soldier/menu.png"},p(_"Soldier")) ..
		--dependencies({"tribes/barbarians/warmill/menu.png","tribes/barbarians/sharpax/menu.png;tribes/barbarians/broadax/menu.png;tribes/barbarians/bronzeax/menu.png;tribes/barbarians/battleax/menu.png;tribes/barbarians/warriorsax/menu.png","tribes/barbarians/trainingcamp/menu.png"}) ..

	--Workers Section
--	building_help_crew_string("barbarians", building_description, {"master-blacksmith", "blacksmith"}, "hammer") ..

	--Building Section
	building_help_building_section("barbarians", building_description, "axfactory", {"metalworks", "axfactory"}) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce each axs in about %s on average.":bformat("57s")) ..
		rt(p(_"All weapons require the same time for making, but the consumption of the same raw materials increases from step to step. The last two need gold."))
   end
}
