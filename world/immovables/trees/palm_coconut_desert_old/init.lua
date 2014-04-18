dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_coconut_desert_old",
   descname = _ "Coconut Palm (Old)",
   -- category = "trees_palm",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree6 36",
         "seed=palm_coconut_desert_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 24, 59 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird3",
         },
      },
   },
}
