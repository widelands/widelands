dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_cloth",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Cloth",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"cloth",
   tribe = "barbarians",
   default_target_quantity = 10,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Cloth is needed for barbarian ships. It is produced out of thatch reed.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 9 },
      },
   }
}
