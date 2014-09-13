dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_armor",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Armor",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"suits of armor",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Basic armor for Empire soldiers. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 1 to level 2.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 11 },
      },
   }
}
