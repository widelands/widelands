push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "debris01",
   descname = _("Debris"),
   size = "small",
   programs = {},
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   animations = {
      idle = {
         hotspot = { 35, 35 },
      },
   }
}

pop_textdomain()
