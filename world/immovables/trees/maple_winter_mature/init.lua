dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "maple_winter_mature",
   descname = _ "Maple (Mature)",
   -- category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 50000",
         "remove=18",
         "grow=maple_winter_old",
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
