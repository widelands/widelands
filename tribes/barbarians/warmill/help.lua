use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian War Mill")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/warmill/warmill_i_00.png", p(
			_[["A new Warriors Axe brings forth the best in its wielder – or the worst in its maker."]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", [[An old Barbarian proverb<br> meaning that you need to take some risks sometimes.]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"The Barbarian War Mill is their most advanced production site for weapons. As such it needs to be upgraded from an Axefactory.") ..
		rt("image=tribes/barbarians/axe/menu.png;tribes/barbarians/sharpaxe/menu.png;tribes/barbarians/broadaxe/menu.png;tribes/barbarians/bronzeaxe/menu.png;tribes/barbarians/battleaxe/menu.png;tribes/barbarians/warriorsaxe/menu.png image-align=right",p("The War Mill produces all the axes that the Barbarians use for warfare.")) ..
		text_line(_"Working radius:", "n/a") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/smelting_works/menu.png","tribes/barbarians/iron/menu.png","tribes/barbarians/warmill/menu.png"}) ..
		rt(p(_"Provided by the " .. _"Smelting Works")) ..
		dependencies({"tribes/barbarians/smelting_works/menu.png","tribes/barbarians/gold/menu.png","tribes/barbarians/warmill/menu.png"}) ..
		rt(p(_"Provided by the " .. _"Smelting Works")) ..
		dependencies({"tribes/barbarians/coalmine/menu.png;tribes/barbarians/burners_house/menu.png","tribes/barbarians/coal/menu.png","tribes/barbarians/warmill/menu.png"}) ..
		rt(p(_"Provided by the " .. _"Coal mine" .. " or the " .. _"Charcoal Burner's House")) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/warmill/menu.png","tribes/barbarians/axe/menu.png","tribes/barbarians/headquarters/menu.png"},p(_"Headquarters")) ..
		dependencies({"tribes/barbarians/warmill/menu.png","tribes/barbarians/sharpaxe/menu.png;tribes/barbarians/broadaxe/menu.png;tribes/barbarians/bronzeaxe/menu.png;tribes/barbarians/battleaxe/menu.png;tribes/barbarians/warriorsaxe/menu.png","tribes/barbarians/trainingscamp/menu.png"}) ..
		rt(p(_"Trainingscamp")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Medium plot","pics/medium.png") ..
		text_line(_"Upgraded from:", _"Axefactory","tribes/barbarians/axefactory/menu.png") ..
		rt(h3(_"Upgrade Cost:")) ..
		image_line("tribes/barbarians/blackwood/menu.png",1,p("1 " .. _"Blackwood")) ..
		image_line("tribes/barbarians/thatchreed/menu.png",1,p("1 " .. _"Thatch Reed")) ..
		image_line("tribes/barbarians/raw_stone/menu.png",2,p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/grout/menu.png",1,p("1 " .. _"Grout")) ..
		image_line("tribes/barbarians/trunk/menu.png",1,p("1 " .. _"Trunk")) ..
		rt(h3(_"Cost cumulative:")) ..
		image_line("tribes/barbarians/blackwood/menu.png",3,p("3 " .. _"Blackwood")) ..
		image_line("tribes/barbarians/thatchreed/menu.png",3,p("3 " .. _"Thatch Reed")) ..
		image_line("tribes/barbarians/raw_stone/menu.png",6,p("6 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/grout/menu.png",3,p("3 " .. _"Grout")) ..
		image_line("tribes/barbarians/trunk/menu.png",3,p("3 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/blackwood/menu.png",2,p("2 " .. _"Blackwood")) ..
		image_line("tribes/barbarians/thatchreed/menu.png",2,p("2 " .. _"Thatch Reed")) ..
		image_line("tribes/barbarians/raw_stone/menu.png",3,p("3 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/grout/menu.png",2,p("2 " .. _"Grout")) ..
		image_line("tribes/barbarians/trunk/menu.png",2,p("2 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/master-blacksmith/menu.png", 1, p("1 " .. _"Master Blacksmith" .. _" and")) ..
		image_line("tribes/barbarians/blacksmith/menu.png", 1, p("1 " .. _"Blacksmith" .. _" or better" .. ".")) ..
		text_line(_"Workers use:",_"Hammer","tribes/barbarians/hammer/menu.png") ..
		rt(h3(_"Experience levels:")) ..
		rt("text-align=right", p(_"Blacksmith" .. _" to " .. _"Master Blacksmith" .. " (24 " .. _"EP" .. ")")) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, a War Mill can produce each item in about 57s on average.") ..
		rt(p(_"All weapons require the same time for making, but the consumption of the same raw materials increases from step to step. The last two need gold."))
}