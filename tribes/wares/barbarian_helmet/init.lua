dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_helmet",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Helmet",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"helmets",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"A helmet is a basic tool to protect warriors. It is produced in the helm smithy and used in the training camp – together with food – to train soldiers from health level 0 to level 1.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 8 },
      },
   }
}
