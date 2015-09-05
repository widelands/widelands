dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "armor_helmet",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Helmet"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "helmets"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 30
	},
   preciousness = {
		empire = 2
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Helmet
		empire = pgettext("empire_ware", "The helmet is the basic defense of a warrior. It is produced in an armor smithy. In combination with a wooden spear, it is the equipment to fit out young soldiers. Helmets are also used in training camps – together with food – to train soldiers from health level 0 to level 1.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 10 },
      },
   }
}
