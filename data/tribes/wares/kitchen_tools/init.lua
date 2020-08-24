dirname = path.dirname(__file__)

tribes:new_ware_type {
   msgctxt = "ware",
   name = "kitchen_tools",
   -- TRANSLATORS: This is a ware name used in lists of wares
   descname = pgettext("ware", "Kitchen Tools"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",

   animations = {
      idle = {
         pictures = path.list_files(dirname .. "idle.png"),
         hotspot = { 8, 8 },
      },
   }
}
