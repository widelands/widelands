dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "oak_summer_old",
   descname = _ "Oak (Old)",
   -- category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 2250000",
         "transform=deadtree2 12",
         "seed=oak_summer_sapling",
      },
      fall = {
         "animate=falling 1200",
         "transform=fallentree",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 24, 60 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird2",
         },
      },
      falling = {
         pictures = path.list_directory(dirname, "f_tree_\\d+.png"),
         player_color_masks = {},
         hotspot = { 10, 60 },
         fps = 10,
      },
   },
}
