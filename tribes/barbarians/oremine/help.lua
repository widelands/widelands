use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Iron Ore Mine")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/deeper_oremine/deeper_oremine_i_00.png", p(
			_[["I look at my own pick wearing away day by day and I realize why my work is important."]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic",_"Quote from an anonymous miner.")) ..
	--General Section
		rt(h2(_"General")) ..
		rt(p(_"Iron Ore Mines exploit only 1/3 of the resource. <br>" .. _"From there on, they will only have a 5% chance of finding iron ore.")) ..
		rt(h3(_"Purpose:")) ..
		image_line("tribes/barbarians/ironore/menu.png", 1, p(_"Dig iron ore out of the ground in mountain terrain.")) ..
		text_line(_"Working radius:", "2") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		image_2_arrow("tribes/barbarians/tavern/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/oremine/menu.png", p(_"Rations from a Tavern")) ..
		image_2_arrow("tribes/barbarians/inn/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/oremine/menu.png", p(_"Rations from an Inn")) ..
		image_2_arrow("tribes/barbarians/big_inn/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/oremine/menu.png", p(_"Rations from a Big Inn")) ..
		rt(h3(_"Outgoing:")) ..
		image_2_arrow("tribes/barbarians/resi_iron2/resi_00.png","tribes/barbarians/oremine/menu.png","tribes/barbarians/ironore/menu.png", p(_"Iron Ore")) ..
		image_arrow("tribes/barbarians/ironore/menu.png","tribes/barbarians/smelting_works/menu.png", p(_"Smelting Works")) ..
		rt(p(_"Iron ore always goes to the Smelting works. It has no other use.")) ..
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
		text_line(_"Upgradeable to:",_"Deep Iron Ore Mine","tribes/barbarians/deep_oremine/menu.png") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/miner/menu.png", 1, p("1 " .. _"Miner" .. _" or better" .. ".")) ..
		text_line(_"Worker uses:",_"Pick","tribes/barbarians/pick/menu.png") ..
		rt(h3(_"Experience levels:")) ..
		rt("text-align=right", p(_"Miner" .. _" to " .. _"Chief Miner" .. " (19 " .. _"EP" .. ")<br>" .. _"Chief Miner" .. _" to " .. _"Master Miner" .. " (28 " .. _"EP" ..")")) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If the food supply is steady, an Iron Ore Mine can produce one iron ore in 65s on average.")
}