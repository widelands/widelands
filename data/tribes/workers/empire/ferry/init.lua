dirname = path.dirname (__file__)

-- TODO(Nordfriese): Make animations
animations = {
   idle = {
      pictures = path.list_files (dirname .. "idle_??.png"),
      hotspot = {29, 25},
   }
}
add_walking_animations (animations, "walk", dirname, "idle", {29, 25})
add_walking_animations (animations, "walkload", dirname, "idle", {29, 25})

tribes:new_ferry_type {
   msgctxt = "empire_worker",
   name = "empire_ferry",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("empire_worker", "Ferry"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animations = animations,
}
