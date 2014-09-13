dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_beer",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Beer",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"beer",
   tribe = "barbarians",
   default_target_quantity = 15,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Beer is produced in micro breweries used in inns and big inns to produce snacks.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 9 },
      },
   }
}
