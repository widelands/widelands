dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_hammer",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Hammer",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"hammers",
   tribe = "atlanteans",
   default_target_quantity = 2,
   preciousness = 1,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The hammer is an essential tool. It is needed by geologists, builders, weaponsmiths and armorsmiths. Make sure you always got some in reserve! It is produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 3, 4 },
      },
   }
}
