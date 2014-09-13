dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_golden_tabard",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Golden Tabard",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"golden tabards",
   tribe = "atlanteans",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Golden tabards are produced in Atlantean weaving mills out of gold yarn. They are used in the labyrinth – together with food – to train soldiers from health level 0 to level 1.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 10 },
      },
   }
}
