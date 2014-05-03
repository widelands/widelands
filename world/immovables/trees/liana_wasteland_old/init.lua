dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "liana_wasteland_old",
   descname = _ "Liana Tree (Old)",
   editor_category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1455000",
         "transform=deadtree4 48",
         "seed=rowan_wasteland_sapling",
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
