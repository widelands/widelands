push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "standing_stone1_desert",
   descname = _("Standing Stone"),
   size = "big",
   programs = {},
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 18, 49 },
      },
   }
}

pop_textdomain()
