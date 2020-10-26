push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "skeleton2",
   descname = _ "Seashell",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 26, 32 },
      },
   }
}

pop_textdomain()
