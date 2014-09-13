dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "imperial_shovel",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Shovel",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"shovels",
   tribe = "empire",
   default_target_quantity = 1,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"The shovel is needed for the proper handling of plants. Therefore the forester and the vinefarmer use it. It is produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 4, 5 },
      },
   }
}
