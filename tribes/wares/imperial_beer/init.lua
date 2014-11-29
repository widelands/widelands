dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_beer",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Beer",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"beer",
   tribe = "empire",
   default_target_quantity = 15,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This beer is produced in a brewery out of wheat and water. It is consumed by miners in coal and iron ore mines.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 9 },
      },
   }
}
