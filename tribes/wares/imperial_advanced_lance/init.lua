dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_advanced_lance",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Advanced Lance",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"advanced lances",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This is an advanced lance with a steel tip. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 1 to level 2.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 10 },
      },
   }
}
