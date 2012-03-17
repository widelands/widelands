use("aux", "formatting")
use("aux", "format_help")

set_textdomain("tribe_barbarians")

return {
	text =
		--rt(h1(_"The Barbarian Ranger's Hut")) ..
	--Lore Section
		rt(h2(_"Lore")) ..
		rt("image=tribes/barbarians/rangers_hut/b_ranger_i_00.png", p(
			--"Wer zwei Halme dort wachsen lässt, wo sonst nur einer wächst, der ist größer als der größte Feldherr!" – Friedrich der Große
			_[["He who can grow two trees where normally grows only one exceeds the most important general!"]])) ..
		rt("text-align=right",p("font-size=10 font-style=italic", _[[%s in a conversation with a %s]]:format("Chat'Karuth",_"Ranger"))) ..
	--General Section
		rt(h2(_"General")) ..
		text_line(_"Purpose:", _"Plant trees in the surrounding area.") ..
		text_line(_"Working radius:", "5") ..
		text_line(_"Conquer range:", "n/a") ..
		text_line(_"Vision range:", "4") ..
	--Dependencies
		rt(h2(_"Dependencies")) ..
		rt(p(_"The Ranger's Hut needs free space in the immediate vicinity to plant the trees.")) ..
		text_line(_"Incoming:", "n/a") ..
		text_line(_"Outgoing:","n/a") ..
	--Building Section
		rt(h2(_"Building")) ..
		text_line(_"Space required:",_"Small plot","pics/small.png") ..
		text_line(_"Upgraded from:", "n/a") ..
		rt(h3(_"Build Cost:")) ..
		image_line("tribes/barbarians/trunk/menu.png", 4, p("4 " .. _"Trunk")) ..
		rt(h3(_"Dismantle yields:")) ..
		image_line("tribes/barbarians/trunk/menu.png", 2, p("2 " .. _"Trunk")) ..
		text_line(_"Upgradeable to:","n/a") ..
	--Workers Section
		rt(h2(_"Workers")) ..
		rt(h3(_"Crew required:")) ..
		image_line("tribes/barbarians/ranger/menu.png", 1, p(_"Ranger")) ..
		text_line(_"Worker uses:",_"Shovel","tribes/barbarians/shovel/menu.png") ..
		text_line(_"Experience levels:","n/a") ..
	--Production Section
		rt(h2(_"Production")) ..
		text_line(_"Performance:", _"The Ranger needs %s to plant a tree, not counting the time he needs to reach the destination and go home again.":format("5s"))
}
