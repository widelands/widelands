dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "deadtree1",
   descname = _"Dead Tree",
   size = "none",
   attributes = {},
   programs = {
      program = {
	     "animate=idle 20000",
		 "remove=16"
      }
   },
   animations = {
      idle = {
         pictures = { dirname .. "/idle.png" },
         player_color_masks = {},
         hotspot = { 25, 56 },
      },
   }
}

