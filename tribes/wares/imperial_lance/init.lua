dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_imperial_lance",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Lance",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"lances",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This lance has an iron spike. It is produced in a weapon smithy and used in a training camp – together with food – to train soldiers from attack level 0 to attack level 1.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 10 },
      },
   }
}
