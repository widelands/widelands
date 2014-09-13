dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_mask",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Mask",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"masks",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This is the most enhanced Barbarian armor. It is produced in the helm smithy and used in the training camp – together with food – to train soldiers from health level 2 to level 3.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 10 },
      },
   }
}
