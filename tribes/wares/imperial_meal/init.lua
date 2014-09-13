dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_meal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Meal",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"meals",
   tribe = "empire",
   default_target_quantity = 20,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"A real meal is made in inns out of bread and fish/meat. It satisfies the needs of miners in deep mines.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 11 },
      },
   }
}
