push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_critter_type{
   name = "duck",
   descname = _ "Duck",
   animation_directory = dirname,
   programs = {
      remove = { "remove" },
   },
   size = 1,
   reproduction_rate = 10,

   animations = {
      idle = {
         sound_effect = {
            path = dirname .. "duck",
            priority = 0.01
         },
         hotspot = { 5, 7 },
         fps = 4,
      },
      walk = {
         hotspot = { 5, 10 },
         directional = true
      }
   }
}

pop_textdomain()
