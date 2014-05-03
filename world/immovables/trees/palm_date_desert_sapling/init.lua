dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_date_desert_sapling",
   descname = _ "Date Palm (Sapling)",
   editor_category = "trees_palm",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 38000",
         "remove=50",
         "grow=palm_date_desert_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}
