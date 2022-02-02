push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "standing_stone5_summer",
   descname = _("Standing Stone"),
   size = "big",
   programs = {},
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 37, 27 },
      },
   }
}

pop_textdomain()
