dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "mushroom_red_wasteland_pole",
   descname = _ "Red Mushroom Tree (Pole)",
   -- category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 40000",
         "remove=25",
         "grow=birch_wasteland_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}
