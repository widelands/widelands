dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "trident_steel",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Steel Trident"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "steel tridents"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 1
	},
   preciousness = {
		atlanteans = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Steel Trident
		atlanteans = pgettext("atlanteans_ware", "This is the medium trident. It is produced in the weapon smithy and used by advanced soldiers in the dungeon to train from attack level 1 to level 2 (together with food).")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
