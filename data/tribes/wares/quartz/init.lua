push_textdomain("tribes")

local dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "quartz",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Quartz"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 7, 13 },
      },
   }
}

pop_textdomain()
