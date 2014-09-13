dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_wheat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Wheat",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"wheat",
   tribe = "empire",
   default_target_quantity = 25,
   preciousness = 12,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Wheat is essential for surviving. It is produced by farms and used by mills and breweries. Donkey farms, sheep farms and piggeries also need to be supplied with wheat.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { -1, 6 },
      },
   }
}
