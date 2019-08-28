dirname = path.dirname (__file__)

animations = {}
add_animation(animations, "idle", dirname .. "tiny", "idle", { 10, 9 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "cocoa_field_tiny",
   descname = _ "Cocoa Field (tiny)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "tiny/menu.png",
   attributes = { "seed_cocoa" },
   programs = {
      program = {
         "animate=idle 80000",
         "remove=10",
         "transform=cocoa_field_small",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname .. "small", "idle", { 19, 21 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "cocoa_field_small",
   descname = _ "Cocoa Field (small)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "small/menu.png",
   attributes = {},
   programs = {
      program = {
         "animate=idle 60000",
         "remove=15",
         "transform=cocoa_field_medium",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname .. "medium", "idle", { 21, 27 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "cocoa_field_medium",
   descname = _ "Cocoa Field (medium)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "medium/menu.png",
   attributes = { "flowering" },
   programs = {
      program = {
         "animate=idle 60000",
         "remove=20",
         "transform=cocoa_field_ripe",
      },
   },
   animations = animations,
}

animations = {}
add_animation(animations, "idle", dirname .. "ripe", "idle", { 21, 27 })

tribes:new_immovable_type {
   msgctxt = "immovable",
   name = "cocoa_field_ripe",
   descname = _ "Cocoa Field (ripe)",
   size = "small",
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "ripe/menu.png",
   attributes = { "ripe_cocoa", "field"},
   programs = {
      program = {
         "animate=idle 500000",
         "remove=",
      },
      harvest = {
         "remove=",
      }
   },
   animations = animations,
}
