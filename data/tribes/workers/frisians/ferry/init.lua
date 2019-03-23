dirname = path.dirname (__file__)

-- TODO(Nordfriese): Make animations
animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {20, 36},
      fps = 10,
      scale = 4,
   }
}
add_walking_animations (animations, "walk", dirname, "idle", {20, 36}, 10, 4)
add_walking_animations (animations, "walkload", dirname, "idle", {20, 36}, 10, 4)

tribes:new_ferry_type {
   msgctxt = "frisians_worker",
   name = "frisians_ferry",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("frisians_worker", "Ferry"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,
}
