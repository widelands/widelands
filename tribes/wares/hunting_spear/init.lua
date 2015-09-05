dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "hunting_spear",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Hunting Spear"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "hunting spears"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		barbarians = 1,
		empire = 1
	},
   preciousness = {
		barbarians = 0,
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Hunting Spear
		default = pgettext("default_ware", "This spear is light enough to be thrown, but heavy enough to kill any animal in one blow. It is only used by hunters."),
		-- TRANSLATORS: Helptext for a ware: Hunting Spear
		barbarians = pgettext("barbarians_ware", "Hunting spears are produced in the metal workshop (but cease to be produced by the building if it is enhanced to an axfactory and war mill)."),
		-- TRANSLATORS: Helptext for a ware: Hunting Spear
		empire = pgettext("empire_ware", "They are produced by the toolsmith.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 5 },
      },
   }
}
