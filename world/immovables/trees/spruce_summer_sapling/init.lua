dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "spruce_summer_sapling",
   descname = _ "Spruce (Sapling)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 55000",
         "remove=42",
         "grow=spruce_summer_pole",
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
