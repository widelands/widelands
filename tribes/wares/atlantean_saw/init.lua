dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_saw",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Saw",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"saws",
   tribe = "atlanteans",
   default_target_quantity = 2,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The saw is needed by the woodcutter and the toolsmith. It is produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 5 },
      },
   }
}
