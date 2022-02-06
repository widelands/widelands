push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "standing_stone4_wasteland",
   descname = _("Standing Stone"),
   size = "big",
   programs = {},
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 11, 74 },
      },
   }
}

pop_textdomain()
