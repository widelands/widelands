dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_plate_armor",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Plate Armor",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"suits of plate armor",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The plate armor is the strongest armor an Empire soldier can have. It is produced in the armor smithy and used in the training camp – together with food – to train soldiers from health level 3 to level 4.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 11 },
      },
   }
}
