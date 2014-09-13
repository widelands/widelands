dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_heavy_double_trident",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Heavy Double Trident",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"heavy double tridents",
   tribe = "atlanteans",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This is the most dangerous weapon of the Atlantean military. Only the best of the best soldiers may use it. It is produced in the weapon smithy and used in the dungeon – together with food – to train soldiers from attack level 3 to level 4.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 10, 17 },
      },
   }
}
