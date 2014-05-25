-- The Barbarian Metal Workshop

include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
   func = function(building_description)
	return

	--Lore Section
	building_help_lore_string("barbarians", building_description, _[[‘We make it work!’]], _[[Inscription on the threshold of the now ruined Olde Forge at Harradsheim, the eldest known smithy.]]) ..

	--General Section
	building_help_general_string("barbarians", building_description, "hammer",
		_"The barbarian metal workshop can make bread paddles, felling axes, fire tongs, fishing rods, hammers, hunting spears, kitchen tools, picks, scythes and shovels.",
		_"The barbarian metal workshop is the basic production site in a series of three buildings and creates all the tools that barbarians need. The others are for weapons.") ..

	--Dependencies
	building_help_inputs("barbarians", building_description) ..

	-- TODO Handle multiple output wares
		rt(h3(_"Outgoing:")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/bread_paddle/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/baker/menu.png" },p(_"Baker")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/felling_ax/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/lumberjack/menu.png" },p(_"Lumberjack")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/fire_tongs/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/smelter/menu.png" },p(_"Smelter")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/fishing_rod/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/fisher/menu.png" },p(_"Fisher")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/hammer/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/builder/menu.png" },p(_"Builder")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/hammer/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/geologist/menu.png" },p(_"Geologist")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/hammer/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/helmsmith/menu.png" },p(_"Helmsmith")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/hammer/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/blacksmith/menu.png" },p(_"Blacksmith")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/hunting_spear/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/hunter/menu.png" },p(_"Hunter")) ..
--		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/kitchen_tools/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/innkeeper/menu.png" },p(_"Innkeeper")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/pick/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/miner/menu.png" },p(_"Miner")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/pick/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/stonemason/menu.png" },p(_"Stonemason")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/scythe/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/farmer/menu.png" },p(_"Farmer")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/shovel/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/ranger/menu.png" },p(_"Ranger")) ..
--		dependencies_basic({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/shovel/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/gardener/menu.png" },p(_"Gardener")) ..


	--Workers Section
	building_help_crew_string("barbarians", building_description, {"blacksmith"}, "hammer") ..

	--Building Section
	building_help_building_section("barbarians", building_description) ..

	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce each tool in about %s on average.":bformat("67s"))
   end
}
