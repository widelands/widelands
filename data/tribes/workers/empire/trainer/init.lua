dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 6, 23 }
   }
}
add_walking_animations(animations, "walk", dirname, "walk", {9, 23}, 10)
add_walking_animations(animations, "walkload", dirname, "walk", {9, 23}, 10)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Trainer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1,
      -- TODO (GunChleoc): Add back in when we have a casern
      --armor_helmet = 1,
      --spear_wooden = 1
   },

   animations = animations,
}
