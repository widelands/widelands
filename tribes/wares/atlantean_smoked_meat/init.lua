dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_smoked_meat",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Smoked Meat",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"smoked meat",
   tribe = "atlanteans",
   default_target_quantity = 20,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"Smoked meat is made out of meat in a smokery. It is delivered to the mines and training sites (labyrinth and dungeon) where the miners and soldiers prepare a nutritious lunch for themselves.",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 16 },
      },
   }
}
