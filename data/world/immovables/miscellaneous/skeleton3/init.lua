push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "skeleton3",
   descname = _ "Skeleton",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 47, 69 },
      },
   }
}

pop_textdomain()
