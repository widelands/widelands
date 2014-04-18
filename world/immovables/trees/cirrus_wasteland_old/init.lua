dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "cirrus_wasteland_old",
   descname = _ "Cirrus Tree (Old)",
   -- category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1455000",
         "transform=deadtree3 34",
         "seed=larch_wasteland_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}
