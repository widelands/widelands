dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = path.glob(dirname, "brownbear_idle_se_*.png"),
      player_color_masks = {},
      hotspot = {21, 16},
      fps = 20
   },
}
add_walking_animations(animations, dirname, "brownbear_walk", {24, 24})

world:new_critter_type{
   name = "brownbear",
   descname = _ "Brown bear",
   swimming = false,
   attributes = { "eatable" },
   programs = {
      remove = { "remove" },
   },
   animations = animations,
}
