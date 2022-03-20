push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "spider_silk",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Spider Silk"),
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   animations = {
      idle = {
         hotspot = { 9, 16 },
      },
   }
}

pop_textdomain()
