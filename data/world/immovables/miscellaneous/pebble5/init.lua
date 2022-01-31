push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "pebble5",
   descname = _("Pebble"),
   size = "none",
   programs = {},
   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 9, 6 },
      },
   }
}

pop_textdomain()
