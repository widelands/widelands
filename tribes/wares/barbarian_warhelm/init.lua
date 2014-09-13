dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_warhelm",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Warhelm",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"warhelms",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"A warhelm is an enhanced armor for barbarian warriors. It is produced in a helm smithy and used in a training camp – together with food – to train soldiers from health level 1 to level 2.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 12 },
      },
   }
}
