dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "mushroom_dark_wasteland_mature",
   descname = _ "Dark Mushroom Tree (Mature)",
   -- category = "trees_wasteland",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 75000",
         "remove=40",
         "grow=oak_wasteland_old",
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
