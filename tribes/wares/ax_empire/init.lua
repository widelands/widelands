dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "ax_empire",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Ax",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"axes",
   default_target_quantity = {
		empire = 3
	},
   preciousness = {
		empire = 1
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Ax
		empire = _"The ax is the tool for the lumberjack. It is produced by the toolsmith."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 3 },
      },
   }
}
