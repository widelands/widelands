push_textdomain("tribes")

dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      hotspot = { 11, 21 }
   }
}
add_directional_animation(animations, "walk", dirname, "walk", {14, 21}, 10)
add_directional_animation(animations, "walkload", dirname, "walk", {14, 21}, 10)


tribes:new_worker_type {
   msgctxt = "europeans_worker",
   name = "europeans_trainer",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext("europeans_worker", "Trainer"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   buildcost = {
      europeans_carrier = 1,
      spear_wooden = 1,
      tabard = 1
   },

   animations = animations,
}

pop_textdomain()
