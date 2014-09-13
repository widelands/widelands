dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_steel_shield",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Steel Shield",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"steel shields",
   tribe = "atlanteans",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This steel shield is produced in the armor smithy and used in the labyrinth – together with food – to train soldiers from defense level 0 to level 1.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
