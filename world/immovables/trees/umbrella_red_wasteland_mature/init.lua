dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "umbrella_red_wasteland_mature",
   descname = _ "Red Umbrella Tree (Mature)",
   -- category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 20000",
         "remove=30",
         "seed=aspen_wasteland_sapling",
         "animate=idle 20000",
         "remove=20",
         "grow=aspen_wasteland_old",
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
