dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "barbarian_sharp_ax",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Sharp Ax",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"sharp axes",
   tribe = "barbarians",
   default_target_quantity = 1,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Young warriors are proud to learn to fight with this powerful weapon. It is heavier and sharper than the ordinary ax. It is produced in axfactories and war mills. In training camps, it is used – together with food – to train soldiers from attack level 0 to level 1.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 7, 7 },
      },
   }
}
