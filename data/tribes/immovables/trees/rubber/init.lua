push_textdomain("tribes")

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
   name = "rubber_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:13.67%",
         "grow=rubber_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   name = "rubber_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m05s",
         "remove=chance:9.37%",
         "grow=rubber_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   name = "rubber_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:1m12s",
         "remove=chance:7.42%",
         "grow=rubber_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   name = "rubber_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   size = "small",
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:25m35s",
         "transform=deadtree7 chance:12.89%",
         "seed=rubber_amazons_sapling proximity:19.53%",
      },
      fall = {
         "animate=fall duration:1s",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

tribes:new_immovable_type{
   name = "rubber_black_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity_black,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:13.67%",
         "grow=rubber_black_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   name = "rubber_black_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity_black,
   programs = {
      main = {
         "animate=idle duration:1m05s",
         "remove=chance:9.37%",
         "grow=rubber_black_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   name = "rubber_black_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity_black,
   programs = {
      main = {
         "animate=idle duration:1m12s",
         "remove=chance:7.42%",
         "grow=rubber_black_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   name = "rubber_black_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   size = "small",
   terrain_affinity = terrain_affinity_black,
   programs = {
      main = {
         "animate=idle duration:25m35s",
         "transform=deadtree7 chance:12.89%",
         "seed=rubber_black_amazons_sapling proximity:19.53%",
      },
      fall = {
         "animate=fall duration:1s",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

tribes:new_immovable_type{
   name = "rubber_desert_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity_desert,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:13.67%",
         "grow=rubber_desert_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   name = "rubber_desert_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity_desert,
   programs = {
      main = {
         "animate=idle duration:1m05s",
         "remove=chance:9.37%",
         "grow=rubber_desert_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   name = "rubber_desert_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity_desert,
   programs = {
      main = {
         "animate=idle duration:1m12s",
         "remove=chance:7.42%",
         "grow=rubber_desert_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   name = "rubber_desert_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   size = "small",
   terrain_affinity = terrain_affinity_desert,
   programs = {
      main = {
         "animate=idle duration:25m35s",
         "transform=deadtree7 chance:12.89%",
         "seed=rubber_desert_amazons_sapling proximity:19.53%",
      },
      fall = {
         "animate=fall duration:1s",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

tribes:new_immovable_type{
   name = "rubber_winter_amazons_sapling",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Sapling)",
   size = "small",
   terrain_affinity = terrain_affinity_winter,
   programs = {
      main = {
         "animate=idle duration:55s",
         "remove=chance:13.67%",
         "grow=rubber_winter_amazons_pole",
      },
   },
   icon = dirname .. "menu_sapling.png",
   animation_directory = dirname,
   spritesheets = spritesheet_sapling
}

tribes:new_immovable_type{
   name = "rubber_winter_amazons_pole",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Pole)",
   size = "small",
   terrain_affinity = terrain_affinity_winter,
   programs = {
      main = {
         "animate=idle duration:1m05s",
         "remove=chance:9.37%",
         "grow=rubber_winter_amazons_mature",
      },
   },
   icon = dirname .. "menu_pole.png",
   animation_directory = dirname,
   spritesheets = spritesheet_pole
}

tribes:new_immovable_type{
   name = "rubber_winter_amazons_mature",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Mature)",
   size = "small",
   terrain_affinity = terrain_affinity_winter,
   programs = {
      main = {
         "animate=idle duration:1m12s",
         "remove=chance:7.42%",
         "grow=rubber_winter_amazons_old",
      },
   },
   icon = dirname .. "menu_mature.png",
   animation_directory = dirname,
   spritesheets = spritesheet_mature
}

tribes:new_immovable_type{
   name = "rubber_winter_amazons_old",
   -- TRANSLATORS:  This is a amazon rare tree.
   descname = _ "Rubber Tree (Old)",
   -- TRANSLATORS:  This is a amazon rare tree.
   species = _ "Rubber Tree",
   size = "small",
   terrain_affinity = terrain_affinity_winter,
   programs = {
      main = {
         "animate=idle duration:25m35s",
         "transform=deadtree7 chance:12.89%",
         "seed=rubber_winter_amazons_sapling proximity:19.53%",
      },
      fall = {
         "animate=fall duration:1s",
         "remove=",
      },
   },
   icon = dirname .. "menu_old.png",
   animation_directory = dirname,
   spritesheets = spritesheet_old
}

pop_textdomain()
