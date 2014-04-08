dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 40, 84 }
   },
}

world:new_immovable_type{
   name = "stones10",
   descname = _ "Stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      program = {
         "animate=idle",
         "transform=stones9"
      }
   },
   animations = animations
}
