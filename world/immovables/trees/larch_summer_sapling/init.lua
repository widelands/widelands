dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "larch_summer_sapling",
   descname = _ "Larch (Sapling)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 60000",
         "remove=44",
         "grow=larch_summer_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 4, 12 },
         fps = 8,
      },
   },
}
