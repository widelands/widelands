push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "standing_stone6",
   descname = _ "Standing Stone",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 7, 63 },
      },
   }
}

pop_textdomain()
