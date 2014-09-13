dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_milking_tongs",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Milking Tongs",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"milking tongs",
   tribe = "atlanteans",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Milking tongs are used by the spider breeder to milk the spiders. They are produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 9 },
      },
   }
}
