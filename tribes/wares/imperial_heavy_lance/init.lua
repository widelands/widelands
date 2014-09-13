dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_heavy_lance",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Heavy Lance",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"heavy lances",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This is a strong lance with a steel-tip and a little blade. It is produced in the weapon smithy and used in the training camp – together with food – train soldiers from attack level 2 to level 3.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 8, 11 },
      },
   }
}
