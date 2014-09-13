dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_advanced_shield",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Advanced Shield",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"advanced shields",
   tribe = "atlanteans",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"These advanced shields are used by the best soldiers of the Atlanteans. They are produced in the armor smithy and used in the labyrinth – together with food – to train soldiers from Defense level 1 to level 2.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
