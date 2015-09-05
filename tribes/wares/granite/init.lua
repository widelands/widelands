dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "granite",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Granite"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "granite"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 20,
		barbarians = 20,
		empire = 30
	},
   preciousness = {
		atlanteans = 5,
		barbarians = 5,
		empire = 0
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Granite
		default = pgettext("default_ware", "Granite is a basic building material."),
		-- TRANSLATORS: Helptext for a ware: Granite
		atlanteans = pgettext("atlanteans_ware", "The Atlanteans produce granite blocks in quarries and crystal mines."),
		-- TRANSLATORS: Helptext for a ware: Granite
		barbarians = pgettext("barbarians_ware", "The Barbarians produce granite blocks in quarries and granite mines."),
		-- TRANSLATORS: Helptext for a ware: Granite
		empire = pgettext("empire_ware", "The Empire produces granite blocks in quarries and marble mines.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 8 },
      },
   }
}
