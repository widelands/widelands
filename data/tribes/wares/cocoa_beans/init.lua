dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "cocoa_beans",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Cocoa Beans"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = {4, 6},
      },
   }
}
