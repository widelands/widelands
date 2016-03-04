set_textdomain("tribes")

test_terrains_resource_descr = lunit.TestCase("Terrains and resources descriptions test")

--  =======================================================
--  ***************** ResourceDescription *****************
--  =======================================================

function test_terrains_resource_descr:test_resource_descr()
   assert_error("Wrong terrain", function() egbase:get_resource_description("XXX") end)
   assert_error("Wrong number of parameters: 2", function()
      egbase:get_resource_description("XXX", "YYY")
   end)
   assert_error("Wrong number of parameters: 3", function()
      egbase:get_resource_description("XXX","YYY","ZZZ")
   end)
end

function test_terrains_resource_descr:test_resource_descname()
   assert_equal("Coal", egbase:get_resource_description("coal").descname)
   assert_equal("Stones", egbase:get_resource_description("stones").descname)
   assert_equal("Water", egbase:get_resource_description("water").descname)
   assert_equal("Fish", egbase:get_resource_description("fish").descname)
end

function test_terrains_resource_descr:test_resource_name()
   assert_equal("coal", egbase:get_resource_description("coal").name)
   assert_equal("stones", egbase:get_resource_description("stones").name)
   assert_equal("water", egbase:get_resource_description("water").name)
   assert_equal("fish", egbase:get_resource_description("fish").name)
end

function test_terrains_resource_descr:test_resource_is_detectable()
   assert_equal(true, egbase:get_resource_description("coal").is_detectable)
   assert_equal(true, egbase:get_resource_description("stones").is_detectable)
   assert_equal(true, egbase:get_resource_description("water").is_detectable)
   assert_equal(false, egbase:get_resource_description("fish").is_detectable)
end

function test_terrains_resource_descr:test_resource_max_amount()
   assert_equal(20, egbase:get_resource_description("coal").max_amount)
   assert_equal(20, egbase:get_resource_description("stones").max_amount)
   assert_equal(50, egbase:get_resource_description("water").max_amount)
   assert_equal(20, egbase:get_resource_description("fish").max_amount)
end

function test_terrains_resource_descr:test_resource_representative_image()
   assert_equal("world/resources/pics/coal4.png",
      egbase:get_resource_description("coal").representative_image)
   assert_equal("world/resources/pics/stones4.png",
      egbase:get_resource_description("stones").representative_image)
   assert_equal("world/resources/pics/water4.png",
      egbase:get_resource_description("water").representative_image)
   assert_equal("world/resources/pics/fish.png",
      egbase:get_resource_description("fish").representative_image)
end

function test_terrains_resource_descr:test_resource_editor_image()
   assert_equal("world/resources/pics/coal1.png",
      egbase:get_resource_description("coal"):editor_image(0))
   assert_equal("world/resources/pics/coal1.png",
      egbase:get_resource_description("coal"):editor_image(5))
   assert_equal("world/resources/pics/coal2.png",
      egbase:get_resource_description("coal"):editor_image(6))
   assert_equal("world/resources/pics/coal2.png",
      egbase:get_resource_description("coal"):editor_image(10))
   assert_equal("world/resources/pics/coal3.png",
      egbase:get_resource_description("coal"):editor_image(15))
   assert_equal("world/resources/pics/coal4.png",
      egbase:get_resource_description("coal"):editor_image(16))
   assert_equal("world/resources/pics/coal4.png",
      egbase:get_resource_description("coal"):editor_image(1000))
end

--  =======================================================
--  ***************** TerrainDescription ******************
--  =======================================================

function test_terrains_resource_descr:test_terrain_descr()
   assert_error("Wrong terrain", function() egbase:get_terrain_description("XXX") end)
   assert_error("Wrong number of parameters: 2",
      function() egbase:get_terrain_description("XXX", "YYY") end)
   assert_error("Wrong number of parameters: 3",
      function() egbase:get_terrain_description("XXX","YYY","ZZZ") end)
end

function test_terrains_resource_descr:test_terrain_descname()
   assert_equal("Meadow 1",
      egbase:get_terrain_description("wiese1").descname)
   assert_equal("Beach",
      egbase:get_terrain_description("wasteland_beach").descname)
   assert_equal("Forested Mountain 2",
      egbase:get_terrain_description("desert_forested_mountain2").descname)
   assert_equal("Water",
      egbase:get_terrain_description("winter_water").descname)
end

function test_terrains_resource_descr:test_terrain_name()
   assert_equal("wiese1", egbase:get_terrain_description("wiese1").name)
   assert_equal("wasteland_beach",
      egbase:get_terrain_description("wasteland_beach").name)
   assert_equal("desert_forested_mountain2",
      egbase:get_terrain_description("desert_forested_mountain2").name)
   assert_equal("winter_water",
      egbase:get_terrain_description("winter_water").name)
end

function test_terrains_resource_descr:test_terrain_default_resource()
   assert_equal("water", egbase:get_terrain_description("wiese1").default_resource.name)
   assert_equal(nil, egbase:get_terrain_description("wasteland_beach").default_resource)
   assert_equal(nil,
      egbase:get_terrain_description("desert_forested_mountain2").default_resource)
   assert_equal("fish",
      egbase:get_terrain_description("winter_water").default_resource.name)
end

function test_terrains_resource_descr:test_terrain_default_resource_amount()
   assert_equal(10, egbase:get_terrain_description("wiese1").default_resource_amount)
   assert_equal(0, egbase:get_terrain_description("wasteland_beach").default_resource_amount)
   assert_equal(0, egbase:get_terrain_description(
      "desert_forested_mountain2").default_resource_amount)
   assert_equal(4, egbase:get_terrain_description(
      "winter_water").default_resource_amount)
end

function test_terrains_resource_descr:test_terrain_editor_category()
   assert_equal("green", egbase:get_terrain_description("wiese1").editor_category.name)
   assert_equal("Summer", egbase:get_terrain_description("wiese1").editor_category.descname)
   assert_equal("wasteland",
      egbase:get_terrain_description("wasteland_beach").editor_category.name)
   assert_equal("Wasteland",
      egbase:get_terrain_description("wasteland_beach").editor_category.descname)
   assert_equal("desert",
      egbase:get_terrain_description("desert_forested_mountain2").editor_category.name)
   assert_equal("Desert",
      egbase:get_terrain_description("desert_forested_mountain2").editor_category.descname)
   assert_equal("winter",
      egbase:get_terrain_description("winter_water").editor_category.name)
   assert_equal("Winter",
      egbase:get_terrain_description("winter_water").editor_category.descname)
end

function test_terrains_resource_descr:test_terrain_fertility()
   assert_near(0.7, egbase:get_terrain_description("wiese1").fertility, 0.01)
   assert_near(0.2, egbase:get_terrain_description("wasteland_beach").fertility, 0.01)
   assert_near(0.5, egbase:get_terrain_description("desert_forested_mountain2").fertility, 0.01)
   assert_near(0.001, egbase:get_terrain_description("winter_water").fertility, 0.0001)
end

function test_terrains_resource_descr:test_terrain_humidity()
   assert_near(0.6, egbase:get_terrain_description("wiese1").humidity, 0.01)
   assert_near(0.4, egbase:get_terrain_description("wasteland_beach").humidity, 0.01)
   assert_near(0.5, egbase:get_terrain_description("desert_forested_mountain2").humidity, 0.01)
   assert_near(0.999, egbase:get_terrain_description("winter_water").humidity, 0.0001)
end

function test_terrains_resource_descr:test_terrain_temperature()
   assert_equal(100, egbase:get_terrain_description("wiese1").temperature)
   assert_equal(60, egbase:get_terrain_description("wasteland_beach").temperature)
   assert_equal(120, egbase:get_terrain_description("desert_forested_mountain2").temperature)
   assert_equal(50, egbase:get_terrain_description("winter_water").temperature)
end

function test_terrains_resource_descr:test_terrain_representative_image()
   assert_equal("world/terrains/pics/green/wiese1_00.png",
      egbase:get_terrain_description("wiese1").representative_image)
   assert_equal("world/terrains/pics/wasteland/strand_00.png",
      egbase:get_terrain_description("wasteland_beach").representative_image)
   assert_equal("world/terrains/pics/desert/forested_mountain2_00.png",
      egbase:get_terrain_description("desert_forested_mountain2").representative_image)
   assert_equal("world/terrains/pics/winter/water_00.png",
      egbase:get_terrain_description("winter_water").representative_image)
end

function test_terrains_resource_descr:test_valid_resources()
   assert_equal("water", egbase:get_terrain_description("wiese1").valid_resources[1].name)
   assert_equal(0, #egbase:get_terrain_description("wasteland_beach").valid_resources)
   assert_equal(4, #egbase:get_terrain_description("desert_forested_mountain2").valid_resources)
   assert_equal("fish", egbase:get_terrain_description("winter_water").valid_resources[1].name)
end
