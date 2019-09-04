dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 95,
   preferred_humidity = 500,
   preferred_fertility = 500,
   pickiness = 10,
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "balsa_amazons_sapling",
   -- TRANSLATORS: This is a amazon rare tree.
   descname = _ "Balsa Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_balsa_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=balsa_amazons_pole",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "sapling/idle_?.png"),
         hotspot = { 5, 12 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "balsa_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Balsa Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=balsa_amazons_mature",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "pole/idle_?.png"),
         hotspot = { 12, 28 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "balsa_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Balsa Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=balsa_amazons_old",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "mature/idle_?.png"),
         hotspot = { 18, 48 },
         fps = 8,
      },
   },
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "balsa_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Balsa Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Balsa Tree",
   icon = dirname .. "old/idle_0.png",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_balsa" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=balsa_amazons_sapling",
      },
      fall = {
         "remove=",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "old/idle_?.png"),
         hotspot = { 24, 60 },
         fps = 10,
      },
   },
}
