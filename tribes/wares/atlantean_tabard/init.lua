dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_tabard",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Tabard",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"tabards",
   tribe = "atlanteans",
   default_target_quantity = 30,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"A tabard and a light trident are the basic equipment for young soldiers. Tabards are produced in the weaving mill.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 10 },
      },
   }
}
