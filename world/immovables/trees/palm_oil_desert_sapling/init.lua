dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_oil_desert_sapling",
   descname = _ "Oil Palm (Sapling)",
   category = "trees_palm",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 42000",
         "remove=32",
         "grow=palm_oil_desert_pole",
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
