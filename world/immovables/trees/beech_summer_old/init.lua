dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "beech_summer_old",
   descname = _ "Beech (Old)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1525000",
         "transform=deadtree2 20",
         "seed=beech_summer_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname, "idle_\\d+.png"),
         hotspot = { 24, 60 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird6",
         },
      },
   },
}
