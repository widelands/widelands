include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian War Mill")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/warmill/warmill_i_00.png", p(
			_[[‘A new Warrior’s Ax brings forth the best in its wielder – or the worst in its maker.’]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[An old Barbarian proverb<br> meaning that you need to take some risks sometimes.]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"The Barbarian War Mill is their most advanced production site for weapons. As such it needs to be upgraded from an Axfactory.") ..
		rt("image=tribes/barbarians/ax/menu.png;tribes/barbarians/sharpax/menu.png;tribes/barbarians/broadax/menu.png;tribes/barbarians/bronzeax/menu.png;tribes/barbarians/battleax/menu.png;tribes/barbarians/warriorsax/menu.png image-align=right",p("The War Mill produces all the axes that the Barbarians use for warfare.")) ..
		text_line(_"Working radius:", "n/a") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(h3(_"Incoming:")) ..
		dependencies({"tribes/barbarians/smelting_works/menu.png","tribes/barbarians/iron/menu.png","tribes/barbarians/warmill/menu.png"},p(_"%s from the Smelting Works":format(_"Iron"))) ..
		dependencies({"tribes/barbarians/smelting_works/menu.png","tribes/barbarians/gold/menu.png","tribes/barbarians/warmill/menu.png"},p(_"%s from the Smelting Works":format(_"Gold"))) ..
		dependencies({"tribes/barbarians/coalmine/menu.png;tribes/barbarians/burners_house/menu.png","tribes/barbarians/coal/menu.png","tribes/barbarians/warmill/menu.png"}) ..
		rt(p(_"Coal from the Coal mine or the Charcoal Burner’s House")) ..
		rt(h3(_"Outgoing:")) ..
		dependencies({"tribes/barbarians/warmill/menu.png","tribes/barbarians/ax/menu.png","tribes/barbarians/headquarters/menu.png","tribes/barbarians/soldier/menu.png"},p(_"Soldier")) ..
		dependencies({"tribes/barbarians/warmill/menu.png","tribes/barbarians/sharpax/menu.png;tribes/barbarians/broadax/menu.png;tribes/barbarians/bronzeax/menu.png;tribes/barbarians/battleax/menu.png;tribes/barbarians/warriorsax/menu.png","tribes/barbarians/trainingcamp/menu.png"}) ..
		rt(p(_"Training Camp")) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Medium plot","pics/medium.png") ..
		text_line(_"Upgraded from:", _"Axfactory","tribes/barbarians/axfactory/menu.png") ..
		rt(h3(_"Upgrade cost:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 1), 1) ..
		help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 1), 1) ..
		help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 1), 1) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 1), 1) ..
		rt(h3(_"Cost cumulative:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 3), 3) ..
		help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 3), 3) ..
		help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 3), 3) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 6), 6) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 3), 3) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "blackwood", ngettext("%i Blackwood", "%i Blackwood", 2), 2) ..
		help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 2), 2) ..
		help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 2), 2) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 3), 3) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 2), 2) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/master-blacksmith/menu.png", 1, p(_"%s and":format(_"Master Blacksmith"))) ..
		image_line("tribes/barbarians/blacksmith/menu.png", 1, p(_"%s or better":format(_"Blacksmith"))) ..
		text_line(_"Workers use:",_"Hammer","tribes/barbarians/hammer/menu.png") ..
		rt(h3(_"Experience levels:")) ..
		rt("text-align=right", p(_"%s to %s (%s EP)":format(_"Blacksmith",_"Master Blacksmith","24"))) ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, this building can produce %s in about %s on average.":format(_"each item","57s")) ..
		rt(p(_"All weapons require the same time for making, but the consumption of the same raw materials increases from step to step. The last two need gold."))
}
