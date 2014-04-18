dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "umbrella_green_wasteland_sapling",
   descname = _ "Green Umbrella Tree (Sapling)",
   category = "trees_wasteland",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 57000",
         "remove=21",
         "grow=maple_wasteland_pole",
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
