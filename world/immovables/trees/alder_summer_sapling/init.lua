dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "alder_summer_sapling",
   descname = _ "Alder (Sapling)",
   -- category = "trees_deciduous",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 57500",
         "remove=21",
         "grow=alder_summer_pole",
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
