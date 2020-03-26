dirname = path.dirname (__file__)

-- TODO(Nordfriese): Make animations
animations = {
   pictures = path.list_files (dirname .. "idle_??.png"),
   hotspot = {20, 36},
}
animations = {
   idle = animations,
   walk_se = animations,
   walk_sw = animations,
   walk_ne = animations,
   walk_nw = animations,
   walk_e = animations,
   walk_w = animations,
   walkload_se = animations,
   walkload_sw = animations,
   walkload_ne = animations,
   walkload_nw = animations,
   walkload_e = animations,
   walkload_w = animations,
}

tribes:new_ferry_type {
   msgctxt = "amazons_worker",
   name = "amazons_ferry",
   -- TRANSLATORS: This is a worker name used in lists of workers
   descname = pgettext ("amazons_worker", "Ferry"),
   helptext_script = dirname .. "helptexts.lua",
   icon = dirname .. "menu.png",
   vision_range = 2,

   animation_directory = dirname,
   animations = animations,
}
