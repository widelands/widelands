dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "iron",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Iron"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "iron"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 20,
		barbarians = 20,
		empire = 20
	},
   preciousness = {
		atlanteans = 4,
		barbarians = 4,
		empire = 4
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Iron
		default = pgettext("default_ware", "Iron is extracted from iron ore in the smelting works."),
		-- TRANSLATORS: Helptext for a ware: Iron
		atlanteans = pgettext("atlanteans_ware", "It is used in the toolsmithy, armor smithy and weapon smithy."),
		-- TRANSLATORS: Helptext for a ware: Iron
		barbarians = pgettext("barbarians_ware", "It is used to produce weapons and tools in the metal workshop, axfactory, war mill and helm smithy."),
		-- TRANSLATORS: Helptext for a ware: Iron
		empire = pgettext("empire_ware", "Weapons, armor and tools are made of iron.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 9 },
      },
   }
}
