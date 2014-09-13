dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_shovel",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Shovel",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"shovels",
   tribe = "atlanteans",
   default_target_quantity = 2,
   preciousness = 0,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Shovels are needed for the proper handling of plants. Therefore the forester and the blackroot farmer are using them. They are produced by the toolsmith.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 5, 8 },
      },
   }
}
