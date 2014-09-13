dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_corn_flour",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Cornflour",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"cornflour",
   tribe = "atlanteans",
   default_target_quantity = 15,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Cornflour is produced in a mill out of corn and is one of three parts of the Atlantean bread produced in bakeries.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 15 },
      },
   }
}
