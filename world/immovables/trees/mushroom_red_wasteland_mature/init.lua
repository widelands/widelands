dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "mushroom_red_wasteland_mature",
   descname = _ "Red Mushroom Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 27000",
         "remove=10",
         "seed=mushroom_red_wasteland_sapling",
         "animate=idle 29000",
         "remove=10",
         "grow=mushroom_red_wasteland_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}
