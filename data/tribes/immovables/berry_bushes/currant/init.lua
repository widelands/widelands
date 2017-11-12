dirname = path.dirname(__file__)

terrain_affinity = {
   -- Temperature is in arbitrary units.
   preferred_temperature = 110,

   -- In percent (1 being very wet).
   preferred_humidity = 0.7,

   -- In percent (1 being very fertile).
   preferred_fertility = 0.85,

   -- A value in [0, 1] that defines how well this can deal with non-ideal
   -- situations. Lower means it is less picky, i.e. it can deal better.
   pickiness = 0.8,
}

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "berry_bush_currant_tiny",
   descname = _ "Currant Bush (Tiny)",
   size = "small",
   attributes = { "bush_tiny" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 22000",
         "transform=berry_bush_currant_small",
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
   name = "berry_bush_currant_small",
   descname = _ "Currant Bush (Small)",
   size = "small",
   attributes = {},
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 28000",
         "transform=berry_bush_currant_medium",
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
   name = "berry_bush_currant_medium",
   descname = _ "Currant Bush (Medium)",
   size = "small",
   attributes = { "flowering" },
   terrain_affinity = terrain_affinity,
   programs = {
      program = {
         "animate=idle 40000",
         "transform=berry_bush_currant_ripe",
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
   name = "berry_bush_currant_ripe",
   descname = _ "Currant Bush (Ripe)",
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
