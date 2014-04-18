dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "aspen_summer_sapling",
   descname = _ "Aspen (Sapling)",
   -- category = "trees_deciduous",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 38000",
         "remove=50",
         "grow=aspen_summer_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}
