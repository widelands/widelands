dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "flour",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Flour"),
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = pgettext("ware", "flour"),
   icon = dirname .. "menu.png",
   default_target_quantity = {
		empire = 20
	},
   preciousness = {
		empire = 5
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Flour
		empire = pgettext("empire_ware", "Flour is produced by the mill out of wheat and is needed in the bakery to produce the tasty Empire bread.")
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 15 },
      },
   }
}
