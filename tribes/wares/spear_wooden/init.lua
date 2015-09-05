dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "spear_wooden",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Wooden Spear"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "wooden spear"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 30
	},
   preciousness = {
		empire = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Wooden Spear
		empire = pgettext("empire_ware", "This wooden spear is the basic weapon in the Empire military system. It is produced in the weapon smithy. In combination with a helmet, it is the equipment to fit out young soldiers.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 11 },
      },
   }
}
