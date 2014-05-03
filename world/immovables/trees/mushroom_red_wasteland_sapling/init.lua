dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "mushroom_red_wasteland_sapling",
   descname = _ "Red Mushroom Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 42000",
         "remove=32",
         "grow=birch_wasteland_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}
