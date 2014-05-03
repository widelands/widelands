dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "liana_wasteland_mature",
   descname = _ "Liana Tree (Mature)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 55000",
         "remove=10",
         "seed=rowan_wasteland_sapling",
         "animate=idle 30000",
         "remove=10",
         "grow=rowan_wasteland_old",
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
