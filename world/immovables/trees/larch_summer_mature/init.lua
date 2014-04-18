dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "larch_summer_mature",
   descname = _ "Larch (Mature)",
   -- category = "trees_coniferous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 55000",
         "remove=24",
         "grow=larch_summer_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 12, 48 },
         fps = 8,
      },
   },
}
