dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "armor",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Armor"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "suits of armor"),
   directory = dirname,
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 0
	},
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 11 },
      },
   }
}
