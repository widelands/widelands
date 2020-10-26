push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "standing_stone4_desert",
   descname = _ "Standing Stone",
   size = "big",
   programs = {},
   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 11, 73 },
      },
   }
}

pop_textdomain()
