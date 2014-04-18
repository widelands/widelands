dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "birch_summer_mature",
   descname = _ "Birch (Mature)",
   category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 25000",
         "remove=10",
         "seed=birch_summer_sapling",
         "animate=idle 30000",
         "remove=10",
         "grow=birch_summer_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 18, 47 },
         fps = 8,
         sfx = {},
      },
   },
}
