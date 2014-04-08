dirname = path.dirname(__file__)

animations = {
   idle = {
      pictures = { dirname .. "idle.png" },
      player_color_masks = {},
      hotspot = { 38, 89 }
   },
}

world:new_immovable_type{
   name = "stones11",
   descname = _ "Stones",
   size = "big",
   attributes = { "stone" },
   programs = {
      program = {
         "animate=idle",
         "transform=stones10"
      }
   },
   animations = animations
}
