dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "alder_summer_mature",
   descname = _ "Alder (Mature)",
   -- category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 50000",
         "remove=18",
         "grow=alder_summer_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}
