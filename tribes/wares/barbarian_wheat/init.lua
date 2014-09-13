dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_wheat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Wheat",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"wheat",
   tribe = "barbarians",
   default_target_quantity = 25,
   preciousness = 12,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Wheat is essential for surviving. It is produced in farms and consumed by bakeries, micro breweries and breweries. Also Cattle farms need to be supplied with wheat.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { -1, 6 },
      },
   }
}
