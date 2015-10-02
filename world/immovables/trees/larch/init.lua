dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 50,
   preferred_humidity = 0.8,
   preferred_fertility = 0.45,
   pickiness = 0.8,
}

world:new_immovable_type{
   name = "larch_summer_sapling",
   descname = _ "Larch (Sapling)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "tree_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "remove=44",
         "grow=larch_summer_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "sapling/", "idle_\\d+.png"),
         hotspot = { 4, 12 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "larch_summer_pole",
   descname = _ "Larch (Pole)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 60000",
         "remove=34",
         "grow=larch_summer_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "pole/", "idle_\\d+.png"),
         hotspot = { 9, 28 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "larch_summer_mature",
   descname = _ "Larch (Mature)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=24",
         "grow=larch_summer_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "mature/", "idle_\\d+.png"),
         hotspot = { 12, 48 },
         fps = 8,
      },
   },
}

world:new_immovable_type{
   name = "larch_summer_old",
   descname = _ "Larch (Old)",
   editor_category = "trees_coniferous",
   size = "small",
   attributes = { "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1455000",
         "transform=deadtree3 23",
         "seed=larch_summer_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_directory(dirname .. "old/", "idle_\\d+.png"),
         hotspot = { 15, 59 },
         fps = 10,
         sound_effect = {
            directory = "sound/animals",
            name = "bird6",
         },
      },
   },
}
