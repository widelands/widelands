use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Trainingscamp")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/trainingscamp/trainingscamp_i_00.png", p(
			_[["He who is strong shall not forgive nor forget, but revenge injustice suffered - in the past and for all future."]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[Chief Chat'Karuth in a speech to his army.]])) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"Trains soldiers in Attack up to level 4, and in Hitpoints up to level 2.<br>Equips the soldiers with all necessary weapons and armour parts.") ..
		rt("_<p font-weight=bold font-decoration=underline font-size=12>Note: </p>_<p font-size=12>Barbarian soldiers cannot be trained in \"Defense\" and will remain at the level with which they came.<br><br></p>") ..
		text_line(_"Working radius:", "n/a") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		dependencies({"tribes/barbarians/soldier/untrained.png","tribes/barbarians/trainingscamp/menu.png","tribes/barbarians/soldier/fulltrained-evade.png"}) ..
		dependencies({"tribes/barbarians/soldier/untrained+evade.png","tribes/barbarians/trainingscamp/menu.png","tribes/barbarians/soldier/fulltrained.png"}) ..
		rt(h3(_"Attack Training:")) ..
		dependencies({"tribes/barbarians/axefactory/menu.png","tribes/barbarians/sharpaxe/menu.png;tribes/barbarians/broadaxe/menu.png","tribes/barbarians/trainingscamp/menu.png"}) ..
		rt(p(_"Provided by the " .. _"Axefactory")) ..
		dependencies({"tribes/barbarians/warmill/menu.png","tribes/barbarians/sharpaxe/menu.png;tribes/barbarians/broadaxe/menu.png;tribes/barbarians/bronzeaxe/menu.png;tribes/barbarians/battleaxe/menu.png;tribes/barbarians/warriorsaxe/menu.png","tribes/barbarians/trainingscamp/menu.png"}) ..
		rt(p(_"Provided by the " .. _"War Mill")) ..
		rt(h3(_"Hitpoints Training:")) ..
		dependencies({"tribes/barbarians/helmsmithy/menu.png","tribes/barbarians/helm/menu.png;tribes/barbarians/mask/menu.png;tribes/barbarians/warhelmet/menu.png","tribes/barbarians/trainingscamp/menu.png"}) ..
		rt(p(_"Provided by the " .. _"Helmsmithy")) ..
		rt(h3(_"Both Trainings:")) ..
		image_line("tribes/barbarians/pittabread/menu.png",1,p(_"%s and":format(_"Pitta Bread"))) ..
		image_line("tribes/barbarians/fish/menu.png;tribes/barbarians/meat/menu.png",1,p(_"%s or %s":format(_"Fish",_"Meat"))) ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Big plot","pics/big.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build Cost:")) ..
		image_line("tribes/barbarians/gold/menu.png",4,p("4 " .. _"Gold")) ..
		image_line("tribes/barbarians/thatchreed/menu.png",3,p("3 " .. _"Thatch Reed")) ..
		image_line("tribes/barbarians/raw_stone/menu.png",4,p("4 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/grout/menu.png",6,p("6 " .. _"Grout")) ..
		image_line("tribes/barbarians/trunk/menu.png",6,p("6 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/gold/menu.png",2,p("2 " .. _"Gold")) ..
		image_line("tribes/barbarians/thatchreed/menu.png",2,p("2 " .. _"Thatch Reed")) ..
		image_line("tribes/barbarians/raw_stone/menu.png",2,p("2 " .. _"Raw Stone")) ..
		image_line("tribes/barbarians/grout/menu.png",3,p("3 " .. _"Grout")) ..
		image_line("tribes/barbarians/trunk/menu.png",3,p("3 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/trainer/menu.png", 1, p(_"Trainer")) ..
		text_line(_"Worker uses:","n/a") ..
		text_line(_"Experience levels:", "n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"If all needed wares are delivered in time, a %s can train one new soldier in %s and %s to the final level in %s on average.":format(_"Trainingscamp",_"Attack",_"Hitpoints","4m40s"))
}
