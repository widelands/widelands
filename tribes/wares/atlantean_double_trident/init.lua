dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_double_trident",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Double Trident",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"double tridents",
   tribe = "atlanteans",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The double trident is one of the best tridents produced by the Atlantean weapon smithy. It is used in a dungeon – together with food – to train soldiers from attack level 2 to level 3.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
