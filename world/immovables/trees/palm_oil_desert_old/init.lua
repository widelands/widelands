dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_oil_desert_old",
   descname = _ "Oil Palm (Old)",
   -- category = "trees_palm",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 800000",
         "transform=deadtree5 50",
         "seed=palm_oil_desert_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         player_color_masks = {},
         hotspot = { 24, 60 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird5",
         },
      },
   },
}
