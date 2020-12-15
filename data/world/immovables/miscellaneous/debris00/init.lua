push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "debris00",
   descname = _ "Debris",
   size = "small",
   programs = {remove = {"remove="}},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 35, 35 },
      },
   }
}

pop_textdomain()
