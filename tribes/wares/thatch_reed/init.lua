dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "thatch_reed",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Thatch Reed",
   -- TRANSLATORS: mass description, e.g. 'The economy needs ...'
   genericname = _"thatch reed",
   default_target_quantity = {
		barbarians = 10
	},
   preciousness = {
		barbarians = 5
	},
   helptext = {
		-- TRANSLATORS: Helptext for a ware: Thatch Reed
		barbarians = _"Thatch reed is produced in a reed yard and used to make the roofs of buildings waterproof."
   },
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 12 },
      },
   }
}
