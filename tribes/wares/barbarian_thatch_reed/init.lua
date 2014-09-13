dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_thatch_reed",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Thatch Reed",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"thatch reed",
   tribe = "barbarians",
   default_target_quantity = 10,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Thatch reed is produced in a reed yard and used to make the roofs of buildings waterproof.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 12 },
      },
   }
}
