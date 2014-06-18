dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "umbrella_red_wasteland_old",
   descname = _ "Red Umbrella Tree (Old)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1050000",
         "transform=deadtree2 32",
         "seed=umbrella_red_wasteland_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}
