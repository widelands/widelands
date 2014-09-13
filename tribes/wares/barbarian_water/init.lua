dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_water",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Water",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"water",
   tribe = "barbarians",
   default_target_quantity = 25,
   preciousness = 8,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Water is the essence of life! It is used in the bakery, the micro brewery and the brewery. The lime kiln and the cattle farm also need to be supplied with water.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 8 },
      },
   }
}
