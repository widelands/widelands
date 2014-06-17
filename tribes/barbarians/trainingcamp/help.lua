include "scripting/formatting.lua"
include "scripting/format_help.lua"

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Training Camp")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/trainingcamp/trainingcamp_i_00.png", p(
			_[[‘He who is strong shall neither forgive nor forget, but revenge injustice suffered – in the past and for all future.’]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Chief Chat’Karuth in a speech to his army.]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"Trains soldiers in Attack up to level 5, and in Health up to level 3.<br>Equips the soldiers with all necessary weapons and armor parts.") ..
		rt("<p font-weight=bold font-decoration=underline font-size=12>" .. _"Note:" .. "</p><p font-size=12>" ..
		   _"Barbarian soldiers cannot be trained in ‘Defense’ and will remain at their initial level."
		   .. "<br><br></p>") ..
		text_line(_"Working radius:", "n/a") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		dependencies({"tribes/barbarians/soldier/untrained.png","tribes/barbarians/trainingcamp/menu.png","tribes/barbarians/soldier/fulltrained-evade.png"}) ..
		dependencies({"tribes/barbarians/soldier/untrained+evade.png","tribes/barbarians/trainingcamp/menu.png","tribes/barbarians/soldier/fulltrained.png"}) ..
		rt(h3(_"Training Attack:")) ..
		dependencies({"tribes/barbarians/axfactory/menu.png","tribes/barbarians/sharpax/menu.png;tribes/barbarians/broadax/menu.png","tribes/barbarians/trainingcamp/menu.png"}) ..
		rt(p(_"Provided by the Axfactory")) ..
		dependencies({"tribes/barbarians/warmill/menu.png","tribes/barbarians/sharpax/menu.png;tribes/barbarians/broadax/menu.png;tribes/barbarians/bronzeax/menu.png;tribes/barbarians/battleax/menu.png;tribes/barbarians/warriorsax/menu.png","tribes/barbarians/trainingcamp/menu.png"}) ..
		rt(p(_"Provided by the War Mill")) ..
		rt(h3(_"Training Health:")) ..
		dependencies({"tribes/barbarians/helmsmithy/menu.png","tribes/barbarians/helm/menu.png;tribes/barbarians/mask/menu.png;tribes/barbarians/warhelm/menu.png","tribes/barbarians/trainingcamp/menu.png"}) ..
		rt(p(_"Provided by the Helm Smithy")) ..
		rt(h3(_"Training Both")) ..
		image_line("tribes/barbarians/pittabread/menu.png",1,p(_"%s and":format(_"Pitta Bread"))) ..
		image_line("tribes/barbarians/fish/menu.png;tribes/barbarians/meat/menu.png",1,p(_"%s or %s":format(_"Fish",_"Meat"))) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Big plot","pics/big.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build cost:")) ..
		help_building_line("barbarians", "gold", ngettext("%i Gold", "%i Gold", 4), 4) ..
		help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 3), 3) ..
		help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 6), 6) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 4), 4) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 6), 6) ..
		rt(h3(_"Dismantle yields:")) ..
		help_building_line("barbarians", "gold", ngettext("%i Gold", "%i Gold", 2), 2) ..
		help_building_line("barbarians", "thatchreed", ngettext("%i Thatch Reed", "%i Thatch Reeds", 2), 2) ..
		help_building_line("barbarians", "grout", ngettext("%i Grout", "%i Grout", 3), 3) ..
		help_building_line("barbarians", "raw_stone", ngettext("%i Raw Stone", "%i Raw Stones", 2), 2) ..
		help_building_line("barbarians", "log", ngettext("%i Log", "%i Logs", 3), 3) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/trainer/menu.png", 1, p(_"Trainer")) ..
		text_line(_"Worker uses:","n/a") ..
		text_line(_"Experience levels:", "n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, a Training Camp can train one new soldier in %1$s and %2$s to the final level in %3$s on average.":bformat(_"Attack",_"Health",_"%1$im%2$is":bformat(4,40)))
}
