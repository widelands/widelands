dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_roystonea_desert_sapling",
   descname = _ "Roystonea regia Palm (Sapling)",
   -- category = "trees_palm",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 57500",
         "remove=21",
         "grow=palm_roystonea_desert_pole",
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
