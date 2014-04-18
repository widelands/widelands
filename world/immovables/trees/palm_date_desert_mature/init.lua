dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_date_desert_mature",
   descname = _ "Date Palm (Mature)",
   -- category = "trees_palm",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 20000",
         "remove=30",
         "seed=palm_date_desert_sapling",
         "animate=idle 20000",
         "remove=20",
         "grow=palm_date_desert_old",
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
