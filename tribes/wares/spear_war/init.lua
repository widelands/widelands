dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "spear_war",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "War Spear"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "war spears"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 1
	},
   preciousness = {
		empire = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: War Spear
		empire = pgettext("empire_ware", "This is the best and sharpest weapon the Empire weaponsmith creates for the warriors. It is used in the training camp – together with food – to train soldiers from attack level 3 to level 4.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 3 },
      },
   }
}
