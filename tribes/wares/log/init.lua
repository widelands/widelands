dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "log",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Log"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "logs"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		atlanteans = 40,
		barbarians = 40,
		empire = 40
	},
   preciousness = {
		atlanteans = 14,
		barbarians = 14,
		empire = 14
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Log
		default = pgettext("default_ware", "Logs are an important basic building material. They are produced by felling trees."),
		-- TRANSLATORS: Helptext for a ware: Log
		atlanteans = pgettext("atlanteans_ware", "Atlanteans use logs also as the base for planks, which are used in nearly every building. Besides the sawmill, the charcoal kiln, the toolsmithy and the smokery also need logs for their work."),
		-- TRANSLATORS: Helptext for a ware: Log
		barbarians = pgettext("barbarians_ware", "Barbarian lumberjacks fell the trees; rangers take care of the supply of trees. Logs are also used in the metal workshop to build basic tools, and in the charcoal kiln for the production of coal. The wood hardener refines logs into blackwood by hardening them with fire."),
		-- TRANSLATORS: Helptext for a ware: Log
		empire = pgettext("empire_ware", "Imperial lumberjacks fell the trees; foresters take care of the supply of trees. Logs are also used by the charcoal kiln, the toolsmithy and the sawmill.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 7 },
      },
   }
}
