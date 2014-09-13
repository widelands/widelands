dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_grout",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Grout",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"grout",
   tribe = "barbarians",
   default_target_quantity = 10,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Raw stone can be processed into grout which provides a solid, non burning building material. Grout is produced in a lime kiln. ",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 12 },
      },
   }
}
