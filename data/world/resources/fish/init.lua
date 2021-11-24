push_textdomain("world")

local pics_dir = path.dirname(__file__) .. "../pics/"

wl.Descriptions():new_resource_type{
   name = "resource_fish",
   descname = _ "Fish",
   max_amount = 20,
   detectable = false,
   timeout_ms = 0,
   timeout_radius = 0,
   representative_image = pics_dir .. "fish.png",
   editor_pictures = {
      [5] = pics_dir .. "fish1.png",
      [10] = pics_dir .. "fish2.png",
      [15] = pics_dir .. "fish3.png",
      [1000] = pics_dir .. "fish4.png",
   }
}

pop_textdomain()
