push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "fallentree",
   descname = _("Felled Tree"),
   animation_directory = path.dirname(__file__),
   icon = dirname .. "menu.png",
   size = "none",
   programs = {
      main = {
         "animate=idle duration:30s",
         "remove="
      }
   },
   animations = {
      idle = {
         hotspot = { 2, 31 },
      },
   }
}

pop_textdomain()
