dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "liana_wasteland_sapling",
   descname = _ "Liana Tree (Sapling)",
   -- category = "trees_wasteland",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 60000",
         "remove=40",
         "grow=rowan_wasteland_pole",
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
