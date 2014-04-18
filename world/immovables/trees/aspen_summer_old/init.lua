dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "aspen_summer_old",
   descname = _ "Aspen (Old)",
   category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1050000",
         "transform=deadtree2 15",
         "seed=aspen_summer_sapling",
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
         hotspot = { 23, 58 },
         fps = 10,
         sfx = {
            [0] = "sound/animals/bird1.ogg",
         },
      },
      falling = {
         pictures = path.list_directory(dirname, "f_tree_\\d+.png"),
         player_color_masks = {},
         hotspot = { 20, 59 },
         fps = 10,
      },
   },
}
