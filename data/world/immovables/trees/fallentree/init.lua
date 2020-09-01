push_textdomain("world")

wl.World():new_immovable_type{
   name = "fallentree",
   descname = _ "Fallen Tree",
   animation_directory = path.dirname(__file__),
   size = "none",
   programs = {
      main = {
         "animate=idle duration:30s",
         "remove="
      }
   },
   animations = {
      idle = {
         hotspot = { 2, 31 },
      },
   }
}

pop_textdomain()
