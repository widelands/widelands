use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Micro Brewery")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/micro-brewery/micro-brewery_i_00.png", p(
			_[["Let the first one drive away the hunger, the second one put you at ease; when you have swallowed up your third one, it's time for the next shift!"]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Widespread toast among Miners]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"The Micro Brewery produces Beer of the lower grade. This beer is a vital component of the Snacks that Inns and Big Inns prepare for Miners in Deep Mines.","tribes/barbarians/beer/menu.png") ..
		text_line(_"Working radius:", "n/a") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/well/menu.png","tribes/barbarians/water/menu.png","tribes/barbarians/micro-brewery/menu.png"},p(_"Water from the Well")) ..
		dependencies({"tribes/barbarians/farm/menu.png","tribes/barbarians/wheat/menu.png","tribes/barbarians/micro-brewery/menu.png"},p(_"Wheat from the Farm")) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/micro-brewery/menu.png","tribes/barbarians/beer/menu.png"},p(_"Beer")) ..
		dependencies({"tribes/barbarians/beer/menu.png","tribes/barbarians/inn/menu.png"},p(_"Inn")) ..
		dependencies({"tribes/barbarians/beer/menu.png","tribes/barbarians/big_inn/menu.png"},p(_"Big Inn")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Medium plot","pics/medium.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build Cost:")) ..
		image_line("tribes/barbarians/blackwood/menu.png",2,p("2 " .. _"Blackwood")) ..
		image_line("tribes/barbarians/raw_stone/menu.png",3,p("3 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/thatchreed/menu.png",2,p("2 " .. _"Thatch Reed")) ..
		image_line("tribes/barbarians/trunk/menu.png",3,p("3 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/blackwood/menu.png",1,p("1 " .. _"Blackwood")) ..
		image_line("tribes/barbarians/raw_stone/menu.png",2,p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/thatchreed/menu.png",1,p("1 " .. _"Thatch Reed")) ..
		image_line("tribes/barbarians/trunk/menu.png",2,p("2 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:",_"Brewery","tribes/barbarians/brewery/menu.png") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/brewer/menu.png", 1, p(_"Brewer")) ..
		text_line(_"Workers use:","n/a") ..
		rt(h3(_"Experience levels:")) ..
		rt("text-align=right", p(_"%s to %s (%s EP)":format(_"Brewer",_"Master Brewer","19"))) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce %s in about %s on average.":format(_"Beer","60s"))
}
