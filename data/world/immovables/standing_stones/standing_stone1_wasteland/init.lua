push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "standing_stone1_wasteland",
   descname = _ "Standing Stone",
   size = "big",
   programs = {},
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 18, 50 },
      },
   }
}

pop_textdomain()
