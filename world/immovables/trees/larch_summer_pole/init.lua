dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "larch_summer_pole",
   descname = _ "Larch (Pole)",
   -- category = "trees_coniferous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 60000",
         "remove=34",
         "grow=larch_summer_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 9, 28 },
         fps = 8,
      },
   },
}
