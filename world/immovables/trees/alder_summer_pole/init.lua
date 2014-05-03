dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "alder_summer_pole",
   descname = _ "Alder (Pole)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 52500",
         "remove=19",
         "grow=alder_summer_mature",
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
