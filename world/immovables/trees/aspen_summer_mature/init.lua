dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "aspen_summer_mature",
   descname = _ "Aspen (Mature)",
   -- category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 20000",
         "remove=30",
         "seed=aspen_summer_sapling",
         "animate=idle 20000",
         "remove=20",
         "grow=aspen_summer_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 18, 47 },
         fps = 8,
      },
   },
}
