dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "armor_chain",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Chain Armor"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "suits of chain armor"),
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
