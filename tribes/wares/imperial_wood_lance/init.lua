dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_wood_lance",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Wood Lance",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"wood lances",
   tribe = "empire",
   default_target_quantity = 30,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This wooden lance is the basic weapon in the Empire military system. It is produced in the weapon smithy. In combination with a helmet, it is the equipment to fit out young soldiers.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 11 },
      },
   }
}
