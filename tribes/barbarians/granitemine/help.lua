include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Granite Mine")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/granitemine/granitemine_i_00.png", p(
			_[[‘I can handle tons of granite, man, but no more of your vain prattle.’]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[This phrase was the reply Rimbert the miner – later known as Rimbert the loner – gave, when he was asked to remain seated on an emergency meeting at Stonford in the year of the great flood. <br> The same man had all the 244 granite blocks ready only a week later, and they still fortify the city’s levee.]])) ..
	--General Section
		rt(h2(_"General")) ..
		rt(p(_"A %1$s exploits all of the resource down to the deepest level. But even after having done so, it will still have a %2$s chance of finding some more %3$s.":bformat(_"Granite Mine","5%",_"Raw Stone") .. _" It cannot be upgraded.")) ..
		rt(h3(_"Purpose:")) ..
		image_line("tribes/barbarians/raw_stone/menu.png", 1, p(_"Carve %s out of the rock in mountain terrain.":format(_"Raw Stone"))) ..
		text_line(_"Working radius:", "2") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/tavern/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/granitemine/menu.png"}, p(_"%s from a Tavern":format(_"Ration"))) ..
		dependencies({"tribes/barbarians/inn/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/granitemine/menu.png"}, p(_"%s from an Inn":format(_"Ration"))) ..
		dependencies({"tribes/barbarians/big_inn/menu.png","tribes/barbarians/ration/menu.png","tribes/barbarians/granitemine/menu.png"}, p(_"%s from a Big Inn":format(_"Ration"))) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/resi_granit2/resi_00.png","tribes/barbarians/granitemine/menu.png","tribes/barbarians/raw_stone/menu.png"}, p(_"Raw Stone")) ..
		dependencies({"tribes/barbarians/raw_stone/menu.png","tribes/barbarians/constructionsite/menu.png"},p(_"Construction Site")) ..
		dependencies({"tribes/barbarians/raw_stone/menu.png","tribes/barbarians/lime_kiln/menu.png"}, p(_"Lime Kiln")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Mine plot","pics/mine.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build cost:")) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 4), 4) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 1), 1) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 2), 2) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/miner/menu.png", 1, p(_"%s or better":format(_"Miner"))) ..
		text_line(_"Worker uses:",_"Pick","tribes/barbarians/pick/menu.png") ..
		rt(h3(_"Experience levels:")) ..
		rt("text-align=right", p(_"%s to %s (%s EP)":format(_"Miner",_"Chief Miner","19") .. "<br>" .. _"%s to %s (%s EP)":format(_"Chief Miner",_"Master Miner","28"))) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If the food supply is steady, this mine can produce %s in %s on average.":format(_"Raw Stone","20s"))
}
