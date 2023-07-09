push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_critter_type{
   name = "duck",
   descname = _("Duck"),
   icon = dirname .. "menu.png",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 1,
   reproduction_rate = 10,

   animations = {
      walk = {
         hotspot = { 5, 10 },
         directional = true
      }
   },
   spritesheets = {
      idle = {
         sound_effect = {
            path = dirname .. "duck",
            priority = "0.01%"
         },
         fps = 4,
         frames = 8,
         rows = 4,
         columns = 2,
         hotspot = { 5, 7 }
      },
   },
}

pop_textdomain()
