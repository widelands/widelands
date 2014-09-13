dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_quartz",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Quartz",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"quartz",
   tribe = "atlanteans",
   default_target_quantity = 5,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"These transparent quartz gems are used to build some exclusive buildings. They are produced in a crystal mine.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 13 },
      },
   }
}
