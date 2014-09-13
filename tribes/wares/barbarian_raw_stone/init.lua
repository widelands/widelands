dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_raw_stone",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Raw Stone",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"raw stones",
   tribe = "barbarians",
   default_target_quantity = 20,
   preciousness = 5,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Raw stones are a basic building material of the barbarians. They are produced in quarries and granite mines.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 8 },
      },
   }
}
