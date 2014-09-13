dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_helmet",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Helmet",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"helmets",
   tribe = "empire",
   default_target_quantity = 30,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The helmet is the basic defense of a warrior. It is produced in an armor smithy. In combination with a wood lance, it is the equipment to fit out young soldiers. Helmets are also used in training camps – together with food – to train soldiers from health level 0 to level 1.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 10 },
      },
   }
}
