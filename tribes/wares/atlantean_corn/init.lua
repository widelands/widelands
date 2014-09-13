dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_corn",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Corn",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"corn",
   tribe = "atlanteans",
   default_target_quantity = 25,
   preciousness = 12,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This corn is processed in the mill into fine cornflour that every Atlantean baker needs for a good bread. Also horse and spider farms need to be provided with corn.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 6, 11 },
      },
   }
}
