push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "bar-ruin03",
   descname = _ "Ruin",
   size = "big",
   programs = {},
   animations = {
      idle = {
         directory = dirname,
         basename = "idle",
         hotspot = { 44, 34 },
      },
   }
}

pop_textdomain()
