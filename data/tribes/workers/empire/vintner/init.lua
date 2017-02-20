dirname = path.dirname(__file__)

-- TODO(GunChleoc): Needs a walkload animation.
animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 12, 24 }
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {12, 24}, 10)
add_walking_animations(animations, "walkload", dirname, "walk", {12, 24}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_vintner",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Vintner"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = animations,
}
