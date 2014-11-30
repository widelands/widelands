dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "ax_bronze",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Bronze Ax",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"bronze axes",
   default_target_quantity = {
		barbarians = 1
	},
   preciousness = {
		barbarians = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Bronze Ax
		barbarians = _"The bronze ax is considered a weapon that is hard to handle. Only skilled warriors can use it. It is produced at the war mill and used in the training camp (it trains – together with food – from attack level 2 to level 3)."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 7 },
      },
   }
}
