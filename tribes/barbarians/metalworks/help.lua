use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Metalwork Shop")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/metalworks/metalworks_i_00.png", p(
			_[["We make it work!"]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Inscription on the threshold of the now ruined Olde Forge at Harradsheim, the eldest known smithy.]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"The Barbarian Metalwork Shop is the basic production site in a series of three buildings and creates all the tools that Barbarians need. The others are for weapons.") ..
		rt("image=tribes/barbarians/bakingtray/menu.png;tribes/barbarians/felling_axe/menu.png;tribes/barbarians/fire_tongs/menu.png;tribes/barbarians/fishing_rod/menu.png;tribes/barbarians/hammer/menu.png;tribes/barbarians/hunting_spear/menu.png;tribes/barbarians/kitchen_tools/menu.png;tribes/barbarians/pick/menu.png;tribes/barbarians/scythe/menu.png;tribes/barbarians/shovel/menu.png image-align=right",p("")) ..
		rt(p(_"It can make Bread Paddles, Felling Axes, Fire Tongs, Fishing Rods, Hammers, Hunting Spears, Kitchen Tools, Picks, Scythes and Shovels.")) ..
		text_line(_"Working radius:", "n/a") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/smelting_works/menu.png","tribes/barbarians/iron/menu.png","tribes/barbarians/metalworks/menu.png"},p(_"%s from the Smelting Works":format(_"Iron"))) ..
		dependencies({"tribes/barbarians/lumberjacks_hut/menu.png","tribes/barbarians/trunk/menu.png","tribes/barbarians/metalworks/menu.png"},p(_"%s from the Lumberjack's hut":format(_"Trunk"))) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/bakingtray/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/baker/menu.png" },p(_"Baker")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/felling_axe/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/lumberjack/menu.png" },p(_"Lumberjack")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/fire_tongs/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/smelter/menu.png" },p(_"Smelter")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/fishing_rod/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/fisher/menu.png" },p(_"Fisher")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/hammer/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/builder/menu.png" },p(_"Builder")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/hammer/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/geologist/menu.png" },p(_"Geologist")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/hammer/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/helmsmith/menu.png" },p(_"Helmsmith")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/hammer/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/blacksmith/menu.png" },p(_"Blacksmith")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/hunting_spear/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/hunter/menu.png" },p(_"Hunter")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/kitchen_tools/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/innkeeper/menu.png" },p(_"Innkeeper")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/pick/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/miner/menu.png" },p(_"Miner")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/pick/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/stonemason/menu.png" },p(_"Stonemason")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/scythe/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/farmer/menu.png" },p(_"Farmer")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/shovel/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/ranger/menu.png" },p(_"Ranger")) ..
		dependencies({"tribes/barbarians/metalworks/menu.png","tribes/barbarians/shovel/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/ferner/menu.png" },p(_"Ferner")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Medium plot","pics/medium.png") ..
		text_line(_"Upgraded from:", _"n/a") ..
		rt(h3(_"Build Cost:")) ..
		image_line("tribes/barbarians/blackwood/menu.png", 1, p("1 " .. _"Blackwood")) ..
		image_line("tribes/barbarians/thatchreed/menu.png", 1, p("1 " .. _"Thatch Reed")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 2, p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/grout/menu.png", 1, p("1 " .. _"Grout")) ..
		image_line("tribes/barbarians/trunk/menu.png", 1, p("1 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/blackwood/menu.png", 1, p("1 " .. _"Blackwood")) ..
		image_line("tribes/barbarians/thatchreed/menu.png", 1, p("1 " .. _"Thatch Reed")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 1, p("1 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/grout/menu.png", 1, p("1 " .. _"Grout")) ..
		image_line("tribes/barbarians/trunk/menu.png", 1, p("1 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:",_"Axefactory","tribes/barbarians/axefactory/menu.png") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/blacksmith/menu.png", 1, p(_"%s or better":format(_"Blacksmith"))) ..
		text_line(_"Worker uses:",_"Hammer","tribes/barbarians/hammer/menu.png") ..
		rt(h3(_"Experience levels:")) ..
		dependencies({"tribes/barbarians/blacksmith/menu.png","tribes/barbarians/master-blacksmith/menu.png"}) ..
		rt("text-align=right", p(_"%s to %s (%s EP)":format(_"Blacksmith",_"Master Blacksmith","24"))) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce %s in about %s on average.":format(_"each item", "67s"))
}
