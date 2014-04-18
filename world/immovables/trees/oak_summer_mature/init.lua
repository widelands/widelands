dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "oak_summer_mature",
   descname = _ "Oak (Mature)",
   -- category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 75000",
         "remove=40",
         "grow=oak_summer_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}
