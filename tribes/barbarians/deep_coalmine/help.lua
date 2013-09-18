use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Deep Coal Mine")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/deep_coalmine/deep_coalmine_i_00.png", p(--text identical to coalmine
			_[[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces. <br>
			Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]]
		)) ..
	--General Section
		rt(h2(_"General")) ..
		rt(p(_"A %s exploits only %s of the resource.<br>From there on, it will only have a 5%% chance of finding %s.":format(_"Deep Coal Mine","2/3",_"Coal"))) ..
		rt(h3(_"Purpose:")) ..
		image_line("tribes/barbarians/coal/menu.png", 1, p(_"Dig %s out of the ground in mountain terrain.":format(_"Coal"))) ..
		text_line(_"Working radius:", "2") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/inn/menu.png","tribes/barbarians/snack/menu.png","tribes/barbarians/deep_coalmine/menu.png"}, p(_"%s from an Inn":format(_"Snack"))) ..
		dependencies({"tribes/barbarians/big_inn/menu.png","tribes/barbarians/snack/menu.png","tribes/barbarians/deep_coalmine/menu.png"}, p(_"%s from a Big Inn":format(_"Snack"))) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/resi_coal2/resi_00.png","tribes/barbarians/deep_coalmine/menu.png","tribes/barbarians/coal/menu.png"}, p(_"Coal")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/axefactory/menu.png"}, p(_"Axefactory")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/warmill/menu.png"}, p(_"War Mill")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/helmsmithy/menu.png"}, p(_"Helmsmithy")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/smelting_works/menu.png"}, p(_"Smelting Works")) ..
		dependencies({"tribes/barbarians/coal/menu.png","tribes/barbarians/lime_kiln/menu.png"}, p(_"Lime Kiln")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Mine plot","pics/mine.png") ..
		text_line(_"Upgraded from:",_"Coal mine","tribes/barbarians/coalmine/menu.png") ..
		rt(h3(_"Upgrade cost:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 2, p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 4, p("4 " .. _"Trunk")) ..
		rt(h3(_"Cost cumulative:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 4, p("4 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 8, p("8 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 2, p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 4, p("4 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:",_"Deeper Coal Mine","tribes/barbarians/deeper_coalmine/menu.png") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/chief-miner/menu.png", 1, p(_"%s or better":format(_"Chief Miner"))) ..
		image_line("tribes/barbarians/miner/menu.png", 1, p(_"%s or better":format(_"Miner"))) ..
		text_line(_"Workers use:",_"Pick","tribes/barbarians/pick/menu.png") ..
		rt(h3(_"Experience levels:")) ..
		rt("text-align=right", p(_"%s to %s (%s EP)":format(_"Miner",_"Chief Miner","19") .. "<br>" .. _"%s to %s (%s EP)":format(_"Chief Miner",_"Master Miner","28"))) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If the food supply is steady, this mine can produce %s in %s on average.":format(_"Coal","19.5s"))
}