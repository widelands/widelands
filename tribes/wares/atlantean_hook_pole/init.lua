dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_hook_pole",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Hook Pole",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"hook poles",
   tribe = "atlanteans",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This hook pole is used by the smoker to suspend all the meat and fish from the top of the smokery. It is created by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 14 },
      },
   }
}
