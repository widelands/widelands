dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "blackwood",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Blackwood"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "blackwood"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 40
	},
   preciousness = {
		barbarians = 10
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Blackwood
		barbarians = pgettext("barbarians_ware", "This fire-hardened wood is as hard as iron and it is used for several buildings. It is produced out of logs in the wood hardener.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 10 },
      },
   }
}
