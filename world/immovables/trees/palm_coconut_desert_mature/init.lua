dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_coconut_desert_mature",
   descname = _ "Coconut Palm (Mature)",
   -- category = "trees_palm",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 60000",
         "remove=23",
         "grow=palm_coconut_desert_old",
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
