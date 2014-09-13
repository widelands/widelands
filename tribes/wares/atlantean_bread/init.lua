dirname = path.dirname(__file__)

tribes:new_ware_type{
   name = "atlantean_bread",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = _"Bread",
   -- TRANSLATORS: generic description, e.g. 'The economy needs ...'
   genericname = _"bread",
   tribe = "atlanteans",
   default_target_quantity = 20,
   preciousness = 2,
   -- TRANSLATORS: Helptext for a ware
   helptext = _"This tasty bread is made in bakeries out of cornflour, blackroot flour and water. It is appreciated as basic food for miners, scouts and soldiers in training sites (labyrinth and dungeon).",
   animations = {
      idle = {
         pictures = { dirname .. "idle.png" },
         hotspot = { 9, 13 },
      },
   }
}
