print "hello"

world:new_resource_type({
   name = "coal",
   descname = _ "Coal",
   max_amount = 20,
   detectable = true,
   editor_pictures = {
      [5] = "world/pics/coal1.png",
      [10] = "world/pics/coal2.png",
      [15] = "world/pics/coal3.png",
      [1000] = "world/pics/coal4.png",
   }
})

world:new_resource_type({
   name = "gold",
   descname = _ "Gold",
   max_amount = 20,
   detectable = true,
   editor_pictures = {
      [5] = "world/pics/gold1.png",
      [10] = "world/pics/gold2.png",
      [15] = "world/pics/gold3.png",
      [1000] = "world/pics/gold4.png",
   }
})

world:new_resource_type({
   name = "iron",
   descname = _ "Iron",
   max_amount = 20,
   detectable = true,
   editor_pictures = {
      [5] = "world/pics/iron1.png",
      [10] = "world/pics/iron2.png",
      [15] = "world/pics/iron3.png",
      [1000] = "world/pics/iron4.png",
   }
})

world:new_resource_type({
   name = "granite",
   descname = _ "Granite",
   max_amount = 20,
   detectable = true,
   editor_pictures = {
      [5] = " world/pics/granite1.png",
      [10] = "world/pics/granite2.png",
      [15] = "world/pics/granite3.png",
      [1000] = "world/pics/granite4.png",
   }
})

world:new_resource_type({
   name = "water",
   descname = _ "Water",
   max_amount = 50,
   detectable = true,
   editor_pictures = {
      [1000] = "world/pics/water_resource.png",
   }
})

world:new_resource_type({
   name = "fish",
   descname = _ "Fish",
   max_amount = 20,
   detectable = false,
   editor_pictures = {
      [1000] = "world/pics/fish_resource.png",
   }
})
