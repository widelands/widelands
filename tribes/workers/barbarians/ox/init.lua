dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 10, 25 },
      fps = 20
   }
}
-- TODO(GunChleoc): Make real oxen animations
add_worker_animations(animations, "walk", dirname, "walk", {21, 31}, 20)
add_worker_animations(animations, "walkload", dirname, "walk", {21, 31}, 10)


tribes:new_carrier_type {
   msgctxt = "barbarians_worker",
   name = "barbarians_ox",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("barbarians_worker", "Ox"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   default_target_quantity = 10,
   ware_hotspot = { -2, 13 },

   animations = animations,
}
