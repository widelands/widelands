dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "trident_heavy_double",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Heavy Double Trident"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "heavy double tridents"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Heavy Double Trident
		atlanteans = pgettext("atlanteans_ware", "This is the most dangerous weapon of the Atlantean military. Only the best of the best soldiers may use it. It is produced in the weapon smithy and used in the dungeon – together with food – to train soldiers from attack level 3 to level 4.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
