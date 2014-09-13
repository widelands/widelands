dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_buckets",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Buckets",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"buckets",
   tribe = "atlanteans",
   default_target_quantity = 2,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Big buckets for the fish breeder – produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
