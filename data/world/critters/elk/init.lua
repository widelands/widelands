dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.list_files(dirname .. "elk_idle_??.png"),
      hotspot = { 15, 27 },
      fps = 20,
      sound_effect = {
         -- Sound files with numbers starting for 10 are generating silence. Remove when we move the sound triggering to programs
         directory = "sound/animals",
         name = "elk",
      },
   },
}
add_walking_animations(animations, "walk", dirname, "elk_walk", {21, 34}, 20)

world:new_critter_type{
   name = "elk",
   descname = _ "Elk",
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
