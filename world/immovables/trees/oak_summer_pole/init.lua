dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "oak_summer_pole",
   descname = _ "Oak (Pole)",
   -- category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 72500",
         "remove=70",
         "grow=oak_summer_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}
