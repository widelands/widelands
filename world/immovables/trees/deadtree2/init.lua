dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "deadtree2",
   descname = _ "Dead Tree",
   category = "trees_dead",
   size = "none",
   attributes = {},
   programs = {
      program = {
	     "animate=idle 20000",
		 "remove=12"
      }
   },
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 27, 56 },
      },
   }
}
