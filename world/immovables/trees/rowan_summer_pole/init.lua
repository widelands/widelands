dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "rowan_summer_pole",
   descname = _ "Rowan (Pole)",
   -- category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 55000",
         "remove=30",
         "grow=rowan_summer_mature",
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
