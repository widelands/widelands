dirname = path.dirname(__file__)

terrain_affinity = {
   -- Temperature is in arbitrary units.
   preferred_temperature = 70,

   -- In percent (1 being very wet).
   preferred_humidity = 0.75,

   -- In percent (1 being very fertile).
   preferred_fertility = 0.1,

   -- A value in [0, 1] that defines how well this can deal with non-ideal
   -- situations. Lower means it is less picky, i.e. it can deal better.
   pickiness = 0.5,
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_blueberry_tiny",
   descname = _ "Blueberry Bush (Tiny)",
   size = "small",
   attributes = { "bush_tiny" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 22000",
         "transform=berry_bush_blueberry_small",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "tiny/idle_??.png"),
         hotspot = { 12, 8 },
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_blueberry_small",
   descname = _ "Blueberry Bush (Small)",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 28000",
         "transform=berry_bush_blueberry_medium",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "small/idle_??.png"),
         hotspot = { 13, 12 }
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_blueberry_medium",
   descname = _ "Blueberry Bush (Medium)",
   size = "small",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 40000",
         "transform=berry_bush_blueberry_ripe",
      },
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "medium/idle_??.png"),
         hotspot = { 13, 14 },
      },
   },
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_blueberry_ripe",
   descname = _ "Blueberry Bush (Ripe)",
   size = "small",
   attributes = { "ripe_bush" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 500000",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "ripe/idle_??.png"),
         hotspot = { 13, 18 },
      },
   },
}
