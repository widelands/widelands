dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_cloth",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Cloth",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"cloth",
   tribe = "empire",
   default_target_quantity = 15,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Cloth is needed to build several buildings. It is also consumed in the armor smithy.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 9 },
      },
   }
}
