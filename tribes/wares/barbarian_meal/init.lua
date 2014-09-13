dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_meal",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Meal",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"meal",
   tribe = "barbarians",
   default_target_quantity = 10,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"A meal is made out of pitta bread, stout and fish/meat in a big inn. This substantial food is exactly what workers in a deeper mine need.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 11 },
      },
   }
}
