dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_borassus_desert_sapling",
   descname = _ "Borassus Palm (Sapling)",
   category = "trees_palm",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 70000",
         "remove=80",
         "grow=palm_borassus_desert_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 5, 12 },
         fps = 8,
         sfx = {},
      },
   },
}
