push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ware_type {
   name = "stonebowl",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Stonebowl"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         hotspot = { 6, 6 },
      }
   },
}

pop_textdomain()
