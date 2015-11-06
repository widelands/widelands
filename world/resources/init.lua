pics_dir = path.dirname(__file__) .. "pics/"

world:new_resource_type{
   name = "coal",
   descname = _ "Coal",
   max_amount = 20,
   detectable = true,
   editor_pictures = {
      [5] = pics_dir .. "coal1.png",
      [10] = pics_dir .. "coal2.png",
      [15] = pics_dir .. "coal3.png",
      [1000] = pics_dir .. "coal4.png",
   }
}

world:new_resource_type{
   name = "gold",
   descname = _ "Gold",
   max_amount = 20,
   detectable = true,
   editor_pictures = {
      [5] = pics_dir .. "gold1.png",
      [10] = pics_dir .. "gold2.png",
      [15] = pics_dir .. "gold3.png",
      [1000] = pics_dir .. "gold4.png",
   }
}

world:new_resource_type{
   name = "iron",
   descname = _ "Iron",
   max_amount = 20,
   detectable = true,
   editor_pictures = {
      [5] = pics_dir .. "iron1.png",
      [10] = pics_dir .. "iron2.png",
      [15] = pics_dir .. "iron3.png",
      [1000] = pics_dir .. "iron4.png",
   }
}

world:new_resource_type{
   name = "stones",
   descname = _ "Stones",
   max_amount = 20,
   detectable = true,
   editor_pictures = {
      [5] = pics_dir .. "stones1.png",
      [10] = pics_dir .. "stones2.png",
      [15] = pics_dir .. "stones3.png",
      [1000] = pics_dir .. "stones4.png",
   }
}

world:new_resource_type{
   name = "water",
   descname = _ "Water",
   max_amount = 50,
   detectable = true,
   editor_pictures = {
      [1000] = pics_dir .. "water.png",
   }
}

world:new_resource_type{
   name = "fish",
   descname = _ "Fish",
   max_amount = 20,
   detectable = false,
   editor_pictures = {
      [5] = pics_dir .. "fish1.png",
      [10] = pics_dir .. "fish2.png",
      [15] = pics_dir .. "fish3.png",
      [1000] = pics_dir .. "fish4.png",
      -- Clutch: The editor chooses the image with the highest number for the
      -- UI. So we keep a nice picture for this purpose at the top of this
      -- list.
      [1001] = pics_dir .. "fish.png",
   }
}
