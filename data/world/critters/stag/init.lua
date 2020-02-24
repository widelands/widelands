dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "idle_??.png"),
      sound_effect = {
         -- Sound files with numbers starting for 10 are generating silence. Remove when we move the sound triggering to programs
         path = "sound/animals/stag",
      },
      hotspot = { 12, 26 },
      fps = 20,
   },
   eating = {
      directory = dirname,
      basename = "idle", -- TODO(Nordfriese): Make animation
      hotspot = { 12, 26 },
      fps = 20,
   }
}

add_directional_animation(animations, "walk", dirname, "walk", {25, 30}, 20)

world:new_critter_type{
   name = "stag",
   descname = _ "Stag",
   editor_category = "critters_herbivores",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
   size = 7,
   reproduction_rate = 30,
   appetite = 30,
   herbivore = {"field"},
}
