dirname = path.dirname(__file__)

terrain_affinity = {
   preferred_temperature = 90,
   preferred_humidity = 600,
   preferred_fertility = 650,
   pickiness = 35,
}

terrain_affinity_black = {
   preferred_temperature = 100,
   preferred_humidity = 150,
   preferred_fertility = 850,
   pickiness = 50,
}

terrain_affinity_desert = {
   preferred_temperature = 140,
   preferred_humidity = 500,
   preferred_fertility = 500,
   pickiness = 50,
}

terrain_affinity_winter = {
   preferred_temperature = 45,
   preferred_humidity = 750,
   preferred_fertility = 400,
   pickiness = 50,
}

spritesheet_sapling = { idle = {
   basename = "rubber_sapling",
   hotspot = {8, 18},
   fps = 2,
   frames = 4,
   columns = 2,
   rows = 2
}}
spritesheet_pole = { idle = {
   basename = "rubber_pole",
   hotspot = {11, 44},
   fps = 2,
   frames = 4,
   columns = 2,
   rows = 2
}}
spritesheet_mature = { idle = {
   basename = "rubber_mature",
   hotspot = {18, 75},
   fps = 2,
   frames = 4,
   columns = 2,
   rows = 2
}}
spritesheet_old = {
   idle = {
      basename = "rubber_old",
      hotspot = {18, 75},
      fps = 2,
      frames = 4,
      columns = 2,
      rows = 2
   },
   fall = {
      basename = "rubber_fall",
      hotspot = {19, 80},
      fps = 10,
      frames = 10,
      columns = 5,
      rows = 2,
      play_once = true
   }
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber_sapling" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=rubber_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=rubber_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=rubber_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber", "tree" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=rubber_amazons_sapling",
      },
      fall = {
         "animate=fall 1000",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_black_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber_sapling" },
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=rubber_black_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_black_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=rubber_black_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_black_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=rubber_black_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_black_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber", "tree" },
   terrain_affinity = terrain_affinity_black,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=rubber_black_amazons_sapling",
      },
      fall = {
         "animate=fall 1000",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_desert_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber_sapling" },
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=rubber_desert_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_desert_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=rubber_desert_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_desert_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=rubber_desert_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_desert_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber", "tree" },
   terrain_affinity = terrain_affinity_desert,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=rubber_desert_amazons_sapling",
      },
      fall = {
         "animate=fall 1000",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_winter_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber_sapling" },
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 55000",
         "remove=35",
         "grow=rubber_winter_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_winter_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 65000",
         "remove=24",
         "grow=rubber_winter_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_winter_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 72000",
         "remove=19",
         "grow=rubber_winter_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   msgctxt = "immovable",
   name = "rubber_winter_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   helptext_script = dirname .. "helptexts.lua",
   size = "small",
   attributes = { "tree_rubber", "tree" },
   terrain_affinity = terrain_affinity_winter,
   programs = {
      program = {
         "animate=idle 1535000",
         "transform=deadtree7 33",
         "seed=rubber_winter_amazons_sapling",
      },
      fall = {
         "animate=fall 1000",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}
