use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Coal Mine")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/coalmine/coalmine_i_00.png", p(
			_[[Ages ago, the Barbarians learned to delve into mountainsides for that black material that feeds their furnaces. <br>
			Wood may serve for a household fire and to keep you warm, but when it comes to working with iron or gold, there is no way around coal.]]
		)) ..
	--General Section
		rt(h2(_"General")) ..
		rt(p(_"Coal Mines exploit only 1/3 of the resource. <br>" .. _"From there on, they will only have a 5% chance of finding coal.")) ..
		rt(h3(_"Purpose:")) ..
		image_line("tribes/barbarians/coal/menu.png", 1, p(_"Dig coal out of the ground in mountain terrain.")) ..
		text_line(_"Working radius:", "2") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		image_2_arrow("tribes/barbarians/tavern/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/coalmine/menu.png", p(_"Rations from a Tavern")) ..
		image_2_arrow("tribes/barbarians/inn/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/coalmine/menu.png", p(_"Rations from an Inn")) ..
		image_2_arrow("tribes/barbarians/big_inn/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/coalmine/menu.png", p(_"Rations from a Big Inn")) ..
		rt(h3(_"Outgoing:")) ..
		image_2_arrow("tribes/barbarians/resi_coal2/resi_00.png","tribes/barbarians/coalmine/menu.png","tribes/barbarians/coal/menu.png", p(_"Coal")) ..
		image_arrow("tribes/barbarians/coal/menu.png","tribes/barbarians/metalworks/menu.png", p(_"Metalwork Shop")) ..
		image_arrow("tribes/barbarians/coal/menu.png","tribes/barbarians/axefactory/menu.png", p(_"Axefactory")) ..
		image_arrow("tribes/barbarians/coal/menu.png","tribes/barbarians/warmill/menu.png", p(_"War Mill")) ..
		image_arrow("tribes/barbarians/coal/menu.png","tribes/barbarians/helmsmithy/menu.png", p(_"Helmsmithy")) ..
		image_arrow("tribes/barbarians/coal/menu.png","tribes/barbarians/smelting_works/menu.png", p(_"Smelting Works")) ..
		image_arrow("tribes/barbarians/coal/menu.png","tribes/barbarians/lime_kiln/menu.png", p(_"Lime Kiln")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Mine plot","pics/mine.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build Cost:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 2, p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 4, p("4 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 1, p("1 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/trunk/menu.png", 2, p("2 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:",_"Deep Coal Mine","tribes/barbarians/deep_coalmine/menu.png") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/miner/menu.png", 1, p("1 " .. _"Miner" .. _" or better" .. ".")) ..
		text_line(_"Worker uses:",_"Pick","tribes/barbarians/pick/menu.png") ..
		rt(h3(_"Experience levels:")) ..
		rt("text-align=right", p(_"Miner" .. _" to " .. _"Chief Miner" .. " (19 " .. _"EP" .. ")<br>" .. _"Chief Miner" .. _" to " .. _"Master Miner" .. " (28 " .. _"EP" ..")")) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If the food supply is steady, a Coal Mine can produce one coal in 32.5s on average.")
}