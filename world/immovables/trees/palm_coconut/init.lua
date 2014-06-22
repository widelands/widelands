dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "palm_coconut_desert_sapling",
   descname = _ "Coconut Palm (Sapling)",
   editor_category = "trees_palm",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 55000",
         "remove=42",
         "grow=palm_coconut_desert_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "sapling/", "idle_\\d+.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "palm_coconut_desert_pole",
   descname = _ "Coconut Palm (Pole)",
   editor_category = "trees_palm",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 55000",
         "remove=33",
         "grow=palm_coconut_desert_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "pole/", "idle_\\d+.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "palm_coconut_desert_mature",
   descname = _ "Coconut Palm (Mature)",
   editor_category = "trees_palm",
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
         pictures = path.list_directory(dirname .. "mature/", "idle_\\d+.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "palm_coconut_desert_old",
   descname = _ "Coconut Palm (Old)",
   editor_category = "trees_palm",
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
         pictures = path.list_directory(dirname .. "old/", "idle_\\d+.png"),
         hotspot = { 24, 59 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird3",
         },
      },
   },
}
