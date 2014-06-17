dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "cirrus_wasteland_sapling",
   descname = _ " Cirrus Tree (Sapling)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 61000",
         "remove=44",
         "grow=cirrus_wasteland_pole",
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
