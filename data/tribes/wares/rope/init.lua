push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ware_type {
   name = "rope",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Rope"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 10, 8 },
      }
   },
}

pop_textdomain()
