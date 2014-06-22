dirname = path.dirname(__file__)

world:new_immovable_type{
   name = "alder_summer_sapling",
   descname = _ "Alder (Sapling)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "seed" },
   programs = {
      program = {
         "animate=idle 57500",
         "remove=21",
         "grow=alder_summer_pole",
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
   name = "alder_summer_pole",
   descname = _ "Alder (Pole)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   programs = {
      program = {
         "animate=idle 52500",
         "remove=19",
         "grow=alder_summer_mature",
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
   name = "alder_summer_mature",
   descname = _ "Alder (Mature)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = {},
   -- NOCOM(#sirver): these are default values and need adjusting
   terrain_affinity = {
   },
   programs = {
      program = {
         "animate=idle 50000",
         "remove=18",
         "grow=alder_summer_old",
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
   name = "alder_summer_old",
   descname = _ "Alder (Old)",
   editor_category = "trees_deciduous",
   size = "small",
   attributes = { "tree" },
   programs = {
      program = {
         "animate=idle 1550000",
         "transform=deadtree4 5",
         "seed=alder_summer_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "old/", "idle_\\d+.png"),
         hotspot = { 23, 59 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird4",
         },
      },
   },
}
