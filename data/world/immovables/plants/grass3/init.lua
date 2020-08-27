push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "grass3",
   descname = _ "Grass",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 11 },
         sound_effect = {
            path = "sound/animals/frog",
            priority = 0.01
         },
      },
   }
}

pop_textdomain()
