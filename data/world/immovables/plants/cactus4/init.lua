push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "cactus4",
   descname = _ "Cactus",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 10, 71 },
      },
   }
}

pop_textdomain()
