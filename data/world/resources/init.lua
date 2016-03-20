pics_dir = path.dirname(__file__) .. "pics/"

world:new_resource_type{
   -- Internal name, must be unique
   name = "coal",
   -- The name that will be used in UI and translated.
   descname = _ "Coal",
   -- Maximum possible amount
   max_amount = 20,
   -- A geologist can find it, otherwise false (see Fish)
   detectable = true,
   -- This represents the resource in menus etc.
   representative_image = pics_dir .. "coal4.png",
   -- Picture that is used to indicate the amount of resource on the map
   -- [5] means amount 0 to 5; next line means amount 6 to 10 and so on
   -- The picture with highest number is additionally used in ui
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
   representative_image = pics_dir .. "gold4.png",
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
   representative_image = pics_dir .. "iron4.png",
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
   representative_image = pics_dir .. "stones4.png",
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
   representative_image = pics_dir .. "water4.png",
   editor_pictures = {
      [10] = pics_dir .."water1.png",
      [20] = pics_dir .."water2.png",
      [30] = pics_dir .."water3.png",
      [1000] = pics_dir .. "water4.png",
   }
}

world:new_resource_type{
   name = "fish",
   descname = _ "Fish",
   max_amount = 20,
   detectable = false,
   representative_image = pics_dir .. "fish.png",
   editor_pictures = {
      [5] = pics_dir .. "fish1.png",
      [10] = pics_dir .. "fish2.png",
      [15] = pics_dir .. "fish3.png",
      [1000] = pics_dir .. "fish4.png",
   }
}
