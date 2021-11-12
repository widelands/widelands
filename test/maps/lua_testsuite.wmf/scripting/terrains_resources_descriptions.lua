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
   assert_equal("Coal", egbase:get_resource_description("resource_coal").descname)
   assert_equal("Stones", egbase:get_resource_description("resource_stones").descname)
   assert_equal("Water", egbase:get_resource_description("resource_water").descname)
   assert_equal("Fish", egbase:get_resource_description("resource_fish").descname)
end

function test_terrains_resource_descr:test_resource_name()
   assert_equal("resource_coal", egbase:get_resource_description("resource_coal").name)
   assert_equal("resource_stones", egbase:get_resource_description("resource_stones").name)
   assert_equal("resource_water", egbase:get_resource_description("resource_water").name)
   assert_equal("resource_fish", egbase:get_resource_description("resource_fish").name)
end

function test_terrains_resource_descr:test_resource_is_detectable()
   assert_equal(true, egbase:get_resource_description("resource_coal").is_detectable)
   assert_equal(true, egbase:get_resource_description("resource_stones").is_detectable)
   assert_equal(true, egbase:get_resource_description("resource_water").is_detectable)
   assert_equal(false, egbase:get_resource_description("resource_fish").is_detectable)
end

function test_terrains_resource_descr:test_resource_max_amount()
   assert_equal(20, egbase:get_resource_description("resource_coal").max_amount)
   assert_equal(20, egbase:get_resource_description("resource_stones").max_amount)
   assert_equal(50, egbase:get_resource_description("resource_water").max_amount)
   assert_equal(20, egbase:get_resource_description("resource_fish").max_amount)
end

function test_terrains_resource_descr:test_resource_representative_image()
   assert_equal("world/resources/coal/../pics/coal4.png",
      egbase:get_resource_description("resource_coal").representative_image)
   assert_equal("world/resources/stones/../pics/stones4.png",
      egbase:get_resource_description("resource_stones").representative_image)
   assert_equal("world/resources/water/../pics/water4.png",
      egbase:get_resource_description("resource_water").representative_image)
   assert_equal("world/resources/fish/../pics/fish.png",
      egbase:get_resource_description("resource_fish").representative_image)
end

function test_terrains_resource_descr:test_resource_editor_image()
   assert_equal("world/resources/coal/../pics/coal1.png",
      egbase:get_resource_description("resource_coal"):editor_image(0))
   assert_equal("world/resources/coal/../pics/coal1.png",
      egbase:get_resource_description("resource_coal"):editor_image(5))
   assert_equal("world/resources/coal/../pics/coal2.png",
      egbase:get_resource_description("resource_coal"):editor_image(6))
   assert_equal("world/resources/coal/../pics/coal2.png",
      egbase:get_resource_description("resource_coal"):editor_image(10))
   assert_equal("world/resources/coal/../pics/coal3.png",
      egbase:get_resource_description("resource_coal"):editor_image(15))
   assert_equal("world/resources/coal/../pics/coal4.png",
      egbase:get_resource_description("resource_coal"):editor_image(16))
   assert_equal("world/resources/coal/../pics/coal4.png",
      egbase:get_resource_description("resource_coal"):editor_image(1000))
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
      egbase:get_terrain_description("summer_meadow1").descname)
   assert_equal("Beach",
      egbase:get_terrain_description("wasteland_beach").descname)
   assert_equal("Forested Mountain 2",
      egbase:get_terrain_description("desert_forested_mountain2").descname)
   assert_equal("Water",
      egbase:get_terrain_description("winter_water").descname)
end

function test_terrains_resource_descr:test_terrain_name()
   assert_equal("summer_meadow1", egbase:get_terrain_description("summer_meadow1").name)
   assert_equal("wasteland_beach",
      egbase:get_terrain_description("wasteland_beach").name)
   assert_equal("desert_forested_mountain2",
      egbase:get_terrain_description("desert_forested_mountain2").name)
   assert_equal("winter_water",
      egbase:get_terrain_description("winter_water").name)
end

function test_terrains_resource_descr:test_terrain_default_resource()
   assert_equal("resource_water", egbase:get_terrain_description("summer_meadow1").default_resource.name)
   assert_equal(nil, egbase:get_terrain_description("wasteland_beach").default_resource)
   assert_equal(nil,
      egbase:get_terrain_description("desert_forested_mountain2").default_resource)
   assert_equal("resource_fish",
      egbase:get_terrain_description("winter_water").default_resource.name)
end

function test_terrains_resource_descr:test_terrain_default_resource_amount()
   assert_equal(10, egbase:get_terrain_description("summer_meadow1").default_resource_amount)
   assert_equal(0, egbase:get_terrain_description("wasteland_beach").default_resource_amount)
   assert_equal(0, egbase:get_terrain_description(
      "desert_forested_mountain2").default_resource_amount)
   assert_equal(4, egbase:get_terrain_description(
      "winter_water").default_resource_amount)
end

function test_terrains_resource_descr:test_terrain_fertility()
   assert_equal(700, egbase:get_terrain_description("summer_meadow1").fertility)
   assert_equal(200, egbase:get_terrain_description("wasteland_beach").fertility)
   assert_equal(500, egbase:get_terrain_description("desert_forested_mountain2").fertility)
   assert_equal(1, egbase:get_terrain_description("winter_water").fertility)
end

function test_terrains_resource_descr:test_terrain_humidity()
   assert_equal(600, egbase:get_terrain_description("summer_meadow1").humidity)
   assert_equal(400, egbase:get_terrain_description("wasteland_beach").humidity)
   assert_equal(500, egbase:get_terrain_description("desert_forested_mountain2").humidity)
   assert_equal(999, egbase:get_terrain_description("winter_water").humidity)
end

function test_terrains_resource_descr:test_terrain_temperature()
   assert_equal(100, egbase:get_terrain_description("summer_meadow1").temperature)
   assert_equal(60, egbase:get_terrain_description("wasteland_beach").temperature)
   assert_equal(120, egbase:get_terrain_description("desert_forested_mountain2").temperature)
   assert_equal(50, egbase:get_terrain_description("winter_water").temperature)
end

function test_terrains_resource_descr:test_terrain_representative_image()
   assert_equal("world/terrains/summer/meadow1/idle.png",
      egbase:get_terrain_description("summer_meadow1").representative_image)
   assert_equal("world/terrains/wasteland/beach/idle.png",
      egbase:get_terrain_description("wasteland_beach").representative_image)
   assert_equal("world/terrains/desert/forested_mountain2/idle.png",
      egbase:get_terrain_description("desert_forested_mountain2").representative_image)
   assert_equal("world/terrains/winter/water/water_00.png",
      egbase:get_terrain_description("winter_water").representative_image)
end

function test_terrains_resource_descr:test_valid_resources()
   assert_equal("resource_water", egbase:get_terrain_description("summer_meadow1").valid_resources[1].name)
   assert_equal(0, #egbase:get_terrain_description("wasteland_beach").valid_resources)
   assert_equal(4, #egbase:get_terrain_description("desert_forested_mountain2").valid_resources)
   assert_equal("resource_fish", egbase:get_terrain_description("winter_water").valid_resources[1].name)
end
