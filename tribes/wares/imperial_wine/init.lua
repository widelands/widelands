dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_wine",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Wine",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"wine",
   tribe = "empire",
   default_target_quantity = 20,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This tasty wine is drunk by the miners working the marble and gold mines. It is produced in a winery. ",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 14 },
      },
   }
}
