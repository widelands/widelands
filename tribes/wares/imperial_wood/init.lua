dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_wood",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Wood",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"wood",
   tribe = "empire",
   default_target_quantity = 40,
   preciousness = 7,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Wood is a basic building material of the Empire. It is also used by the weapon smithy.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 12 },
      },
   }
}
