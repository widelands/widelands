dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_long_trident",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Long Trident",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"long tridents",
   tribe = "atlanteans",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The long trident is the first trident in the training of soldiers. It is produced in the weapon smithy and used in the dungeon – together with food – to train soldiers from attack level 0 to level 1. ",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
