dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_stone",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Stones",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"stones",
   tribe = "empire",
   default_target_quantity = 30,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Stones are a basic building material of the Empire. They are produced in quarries and marble mines.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 8 },
      },
   }
}
