dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 16, 29 },
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {26, 30}, 20)
add_directional_animation(animations, "walkload", dirname, "walk", {26, 30}, 20)


tribes:new_worker_type {
   msgctxt = "empire_worker",
   name = "empire_shepherd",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("empire_worker", "Shepherd"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      empire_carrier = 1
   },

   animations = animations,
}
