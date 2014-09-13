dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_fishing_net",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Fishing Net",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"fishing nets",
   tribe = "atlanteans",
   default_target_quantity = 2,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The fishing net is used by the fisher and produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
