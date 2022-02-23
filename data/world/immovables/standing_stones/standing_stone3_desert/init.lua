push_textdomain("world")

dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "standing_stone3_desert",
   descname = _("Standing Stone"),
   size = "big",
   programs = {},
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   animations = {
      idle = {
         hotspot = { 25, 28 },
      },
   }
}

pop_textdomain()
