dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "umbrella_green_wasteland_mature",
   descname = _ "Green Umbrella Tree (Mature)",
   -- category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 50000",
         "remove=18",
         "grow=maple_wasteland_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}
