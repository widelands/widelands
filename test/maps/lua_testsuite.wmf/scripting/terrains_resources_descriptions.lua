set_textdomain("tribes")

test_descr = lunit.TestCase("Terrains and resources descriptions test")

--  =======================================================
--  ***************** ResourceDescription *****************
--  =======================================================

function test_descr:test_resource_descr()
   assert_error("Wrong terrain", function() egbase:get_resource_description("XXX") end)
   assert_error("Wrong number of parameters: 2", function() egbase:get_resource_description("XXX", "YYY") end)
   assert_error("Wrong number of parameters: 3", function() egbase:get_resource_description("XXX","YYY","ZZZ") end)
end


function test_descr:test_descname()
   assert_equal(_"Coal", egbase:get_resource_description("coal").descname)
   assert_equal(_"Stones", egbase:get_resource_description("stones").descname)
   assert_equal(_"Water", egbase:get_resource_description("water").descname)
   assert_equal(_"Fish", egbase:get_resource_description("fish").descname)
end

function test_descr:test_name()
   assert_equal("coal", egbase:get_resource_description("coal").name)
   assert_equal("stones", egbase:get_resource_description("stones").name)
   assert_equal("water", egbase:get_resource_description("water").name)
   assert_equal("fish", egbase:get_resource_description("fish").name)
end

function test_descr:test_is_detectable()
   assert_equal(true, egbase:get_resource_description("coal").is_detectable)
   assert_equal(true, egbase:get_resource_description("stones").is_detectable)
   assert_equal(true, egbase:get_resource_description("water").is_detectable)
   assert_equal(false, egbase:get_resource_description("fish").is_detectable)
end

function test_descr:test_max_amount()
   assert_equal(20, egbase:get_resource_description("coal").max_amount)
   assert_equal(20, egbase:get_resource_description("stones").max_amount)
   assert_equal(50, egbase:get_resource_description("water").max_amount)
   assert_equal(20, egbase:get_resource_description("fish").max_amount)
end

function test_descr:test_representative_image()
   assert_equal("world/resources/pics/coal4.png", egbase:get_resource_description("coal").representative_image)
   assert_equal("world/resources/pics/stones4.png", egbase:get_resource_description("stones").representative_image)
   assert_equal("world/resources/pics/water4.png", egbase:get_resource_description("water").representative_image)
   assert_equal("world/resources/pics/fish.png", egbase:get_resource_description("fish").representative_image)
end

function test_descr:test_editor_image()
   assert_equal("world/resources/pics/coal1.png", egbase:get_resource_description("coal").editor_image(0))
   assert_equal("world/resources/pics/coal1.png", egbase:get_resource_description("coal").editor_image(5))
   assert_equal("world/resources/pics/coal1.png", egbase:get_resource_description("coal").editor_image(9))
   assert_equal("world/resources/pics/coal2.png", egbase:get_resource_description("coal").editor_image(10))
   assert_equal("world/resources/pics/coal3.png", egbase:get_resource_description("coal").editor_image(15))
   assert_equal("world/resources/pics/coal4.png", egbase:get_resource_description("coal").editor_image(16))
   assert_equal("world/resources/pics/coal4.png", egbase:get_resource_description("coal").editor_image(1000))
end
