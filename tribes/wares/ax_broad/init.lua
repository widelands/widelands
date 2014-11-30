dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "ax_broad",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Broad Ax",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"broad axes",
   default_target_quantity = {
		barbarians = 1
	},
   preciousness = {
		barbarians = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Broad Ax
		barbarians = _"The broad ax is produced by the axfactory and the war mill. It is used in the training camp – together with food – to train warriors with a fundamental fighting knowledge (from attack level 1 to attack level 2)."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 6 },
      },
   }
}
