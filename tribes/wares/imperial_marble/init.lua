dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_marble",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Marble",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"marble",
   tribe = "empire",
   default_target_quantity = 30,
   preciousness = 3,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Marble is the beautiful stone which is cut out of the mountains or produced in a quarry. Marble is used as building material and gets chiselled into marble columns in the stonemason’s house.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 18 },
      },
   }
}
