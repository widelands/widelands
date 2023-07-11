push_textdomain("world")

local dirname = path.dirname(__file__)

wl.Descriptions():new_immovable_type{
   name = "blackland_rocks6",
   descname = _("Rocks 6"),
   animation_directory = dirname,
   icon = dirname .. "menu6.png",
   size = "big",
   programs = {
      shrink = {
         "transform=blackland_rocks5"
      }
   },
   animations = {
      idle = {
         basename = "rocks6",
         hotspot = { 30, 75 }
      },
   }
}

pop_textdomain()
