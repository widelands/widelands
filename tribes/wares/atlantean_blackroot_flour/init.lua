dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_blackroot_flour",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Blackroot Flour",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"blackroot flour",
   tribe = "atlanteans",
   default_target_quantity = 15,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Blackroot Flour is produced in mills out of blackroots. It is used in bakeries to make a tasty bread.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 15 },
      },
   }
}
