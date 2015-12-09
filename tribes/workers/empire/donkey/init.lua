dirname = path.dirname(__file__)

animations = {
   idle = {
      template = "idle_??",
      directory = dirname,
      hotspot = { 14, 20 },
      fps = 10
   }
}
add_worker_animations(animations, "walk", dirname, "walk", {14, 25}, 10)
add_worker_animations(animations, "walkload", dirname, "walk", {14, 25}, 10) -- TODO(GunChleoc): Make animation


tribes:new_carrier_type {
   msgctxt = "empire_worker",
   name = "empire_donkey",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Donkey"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   default_target_quantity = 10,
   ware_hotspot =  { -2, 8 },

   animations = animations,
}
