dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "mushroom_dark_wasteland_old",
   descname = _ "Dark Mushroom Tree (Old)",
   category = "trees_wasteland",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1050000",
         "transform=deadtree2 25",
         "seed=oak_wasteland_sapling",
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
         fps = 15,
         sfx = {},
      },
   },
}
