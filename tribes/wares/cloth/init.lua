dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "cloth",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cloth"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "cloth"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 10,
		empire = 15
	},
   preciousness = {
		barbarians = 0,
		empire = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Cloth
		barbarians = pgettext("barbarians_ware", "Cloth is needed for Barbarian ships. It is produced out of thatch reed."),
		-- TRANSLATORS: Helptext for a ware: Cloth
		empire = pgettext("empire_ware", "Cloth is needed to build several buildings. It is also consumed in the armor smithy.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 9 },
      },
   }
}
