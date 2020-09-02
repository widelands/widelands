push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "bar-ruin01",
   descname = _ "Ruin",
   size = "big",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 64, 53 },
      },
   }
}

pop_textdomain()
