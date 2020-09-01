push_textdomain("world")

dirname = path.dirname(__file__)

wl.World():new_immovable_type{
   name = "snowman",
   descname = _ "Snowman",
   size = "none",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 9, 24 },
      },
   }
}

pop_textdomain()
