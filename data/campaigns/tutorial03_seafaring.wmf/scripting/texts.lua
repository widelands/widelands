-- =======================================================================
--                      Texts for the tutorial mission
-- =======================================================================

-- =========================
-- Some formating functions
-- =========================

include "scripting/richtext_scenarios.lua"


-- =============
-- Texts below
-- =============

intro_south = {
   position = "topright",
   field = sf,
   title = _"Seafaring",
   body = (
      h1(_"Seafaring Tutorial") ..
      p(_[[Welcome back. In this tutorial, you are going to learn the most important things about seafaring, that is ships, ports, and expedition.]]) ..
      p(_[[But let me first give you an overview about your territory: here in the south, you have a whole economy with almost everything you need.]])
   ),
   h = 250
}

intro_north = {
   position = "topright",
   field = castle_field,
   title = _"The Northern Part",
   body = (
      p(_[[Here in the northern part, you only have a goldmine and a warehouse. While the miners are supplied well with food, there is no way to transport the gold ore to our smelting works in the southern part.]]) ..
      p(_[[We have tried to build a road, but the mountain is too wide and too steep. We therefore have only one possibility: we need to establish a sea lane between these two parts.]]) ..
      p(_[[But I don’t want to rush you: you have just arrived here and you would probably like to have a closer look at your camp. I will also take a short break and be back soon.]])
   ),
   h = 350
}

tell_about_port = {
   position = "topright",
   title = _"Ports",
   body = (
      h1(_"Ports") ..
      p(_[[For everything you do on the high seas, you need a port at the shore. Ports are like headquarters: they can store wares, workers and soldiers. The soldiers inside will automatically come out when an enemy attacks the port.]]) ..
      p(_[[Additionally, ports offer the possibility of transporting wares via ships. When you click on the port you already have, you will notice two additional tabs: ‘Wares waiting to be shipped’ and ‘workers waiting to embark.’ They are waiting for a ship to transport them to another port. Currently, there are none because we have not yet built a second port. So let’s change this!]])
   ),
   h = 350
}

tell_about_port_building = {
   position = "topright",
   field = second_port_field,
   title = _"Building ports",
   body = (
      h1(_"How to build a port") ..
      p(_[[Ports are big buildings, but they can only be built at special locations: those marked with the]]) ..
      li_image("images/wui/overlays/port.png", _[[blue port space icon.]]) ..
      p(_[[Port spaces are set by the map designer, so a map will either contain them or not. They might, however, be hidden under trees or be blocked by surrounding buildings.]]) ..
      p(_[[You might already have noticed that you have such an icon next to your castle.]]) ..
      li(_[[Build a port in the northern part of your camp.]])
   ),
   obj_name = "build_port",
   obj_title = _"Build a port in the northern part of your camp.",
   obj_body = (
      h1(_"Build a port") ..
      p(_[[You always need a port when you want to transport wares with a ship.]]) ..
      li(_[[Build a port next to your castle, on the blue port space icon.]]) ..
      li_arrow(_[[Ports are built like normal buildings, but are only available on blue port spaces. Just click on a field with the icon and the building menu automatically offers you to build a port.]]) ..
      li_arrow(_[[Although ports act as warehouses, you should not build more than necessary: they cost quartz, diamonds and gold, which makes them quite expensive.]])
   ),
}

tell_about_shipyard = {
   position = "topright",
   title = _"Constructing ships",
   body = (
      h1(_"Let’s build ships") ..
      p(_[[Great. Your port has just been finished. Now we need some ships.]]) ..
      p(_[[Ships are constructed in a shipyard by a shipwright. We have to build one somewhere close to the shore.]]) ..
      li(_[[Build a shipyard close to the coast. It is a medium building.]])
   ),
   h = 300,
   obj_name = "build_shipyard",
   obj_title = _"Build a shipyard",
   obj_body = (
      h1(_"Build a shipyard") ..
      p(_[[Ships are produced in a shipyard.]]) ..
      li(_[[Build a shipyard close to the shore of the southern part of your territory.]]) ..
      li_arrow(_[[The shipyard is a medium building. Although it can be built everywhere on the map, the shipwright only works when he is close to the water and there are no trees or roads at the shoreline.]])
   ),
}

tell_about_ships = {
   position = "topright",
   title = _"Constructing ships",
   body =
      h1(_"Waiting for the ships") ..
      p(_[[Very good. Your shipyard is finished and your shipwright immediately started working. For the construction of ships, he needs logs, planks and spidercloth, which will be transported to the shipyard.]] .. " " ..
      _[[The shipwright will take the ware he needs to a free spot at the shoreline and build a ship there. When the first ship is finished, it will launch onto the sea, and the shipwright will construct another one.]]) ..
      li(_[[We should wait until we have two ships. That should be enough for now.]]) ..
      li_arrow(_[[You need to stop your shipyard when you have enough ships. Otherwise, your shipwright will consume all your logs and spidercloth, producing dozens of ships.]]) ..
      li_image("images/ui_basic/stop.png", _[[This is the icon for stopping production. You will find it in the building window.]]),
   obj_name = "wait_for_ships",
   obj_title = _"Construct two ships",
   obj_body = (
      p(_[[Ships are constructed automatically when the shipyard is complete and the needed wares have been delivered.]]) ..
      li(_[[Wait until the shipwright has constructed two ships.]]) ..
      li_arrow(_[[Do not forget to stop your shipyard when you have enough ships.]])
   )
}

expedition1 = {
   position = "topright",
   title = _"No Iron",
   body = (
      h1(_"We lack iron") ..
      p(_[[The second ship might not be finished yet, but we have an urgent problem.]]) ..
      p(_[[As you surely have already noticed, there is no iron in the mountain in the west. We have plenty of coal and gold ore, but without iron ore, we cannot produce any tools.]]) ..
      p(_[[Although it might take long and be expensive and not without dangers – who knows what monsters live in the sea? – I see no other possibility: we will have to undertake an expedition to the unknown seas.]])
   ),
   h = 300
}

expedition2 = {
   position = "topright",
   -- TRANSLATORS: This shall be the beginning of a poem
   title = _"A trip by the sea, what fun it can be",
   body = (
      h1(_"Expeditions") ..
      p(_[[During an expedition, you send a ship out to discover new islands and maybe found a colony there.]]) ..
      li_image("images/wui/buildings/start_expedition.png",
         join_sentences(_[[Expeditions can be started in every port. Then, all needed wares are transported to that port. The wares are exactly those your tribe needs to build a port (your goal is to build a port far away from home, so this is not surprising), and of course you need a builder, too. When everything is prepared, a ship will come and pick it up.]],
         _[[You can check out the needed wares in the fifth tab of your port (it will appear when you’ve started an expedition).]])) ..
      p(_[[Now try this out. I will tell you later what the next steps are.]]) ..
      li(_[[Start an expedition in any of your ports.]])
   ),
   obj_name = "start_expedition",
   obj_title = _"Start an expedition",
   obj_body = (
      li(_[[Start an expedition.]]) ..
      li_image("images/wui/buildings/start_expedition.png", _[[To do so, click on the ‘Start Expedition’ button in any port. A new tab where you can see the needed wares will appear.]])
   )
}

expedition3 = {
   position = "topright",
   title = _"Off to greener pastures",
   body = (
      h1(_"Start your expedition") ..
      p(join_sentences(
         _[[Your expedition ship is ready. It is waiting for your orders in front of your port. It isn’t transporting wares anymore. Use its buttons to send your ship in any of the six main directions of the Widelands map. When it has reached a coastline, you can make it travel around the coast, where it will look for suitable places for landing.]],
         _[[Once a port space has been found, you can construct a new port with the button in the center of the ship’s control window.]])) ..
      p(_[[The wares will then be unloaded, and the ship will take up the task of transporting wares once again. The builder will start his work and build a port.]]) ..
      li(_[[Search for an island with a mountain, and look for a port space there. Colonize the island.]])
   ),
   obj_name = "found_settlement",
   obj_title = _"Found a settlement",
   obj_body = (
      li(_[[Navigate your ship to an island that could contain iron ore.]]) ..
      li_arrow(_[[When you click on the expedition ship, a window opens where you can control your ship.]]) ..
      li(_[[When you have found a suitable port space, build a port there.]])
   )
}

conclusion = {
   position = "topright",
   field = port_on_island,
   title = _"Conclusion",
   body = (
      h1(_"Congratulations") ..
      p(_[[You’ve lead the expedition to a successful end and founded a new colony. I’ve sent out some geologists – they already report that they’ve found some iron ore.]]) ..
      p(_[[So far you have learned everything about seafaring: how to build ports and ships and how to send out an expedition. Remember that expeditions are sometimes the fastest way to reach essential resources – and sometimes the only one.]]) ..
      p(_[[But I want to speak a word of warning. Ports are like headquarters: they can be attacked by a nearby enemy. While your headquarters has soldiers to defend it, your newly built port won’t. You should therefore avoid settling next to an enemy.]]) ..
      p(_[[On this map, there is no enemy to fear. In other games, you should make building one or two military fortifications around your new colonies a priority.]])
   ),
   h = 450
}

ferry_1 = {
   position = "topright",
   field = waterway_field,
   title = _"There is more to it…",
   body = (
      h1(_"Another gold mountain") ..
      p(_[[Now that you have learned all about ships, there is another way of water-based transport I would like to teach you.]]) ..
      p(_[[Ships have the advantage that they can carry large quantities of wares and workers at a time, but unfortunately their destinations are limited to a handful of spaces suited for a port.]]) ..
      p(_[[Look at this valley here in the far south. We have found a mountain with gold down here and would like to mine ores from it as well. But the streams seperating it from our main colony are too wide to build bridges and too narrow for ships to pass them. Our roads would have to take long curves, which slows down ware transport a lot.]])
   ),
   h = 250
}
ferry_2 = {
   position = "topright",
   title = _"There is more to it…",
   body = (
      h1(_"Rowboats") ..
      p(_[[This is where ferries come in. Ferries are small rowboats that can carry only one ware at a time, and no workers (other than the one who rows it and who never leaves his boat). They can travel between any two flags located on the shore.]]) ..
      li_image("images/wui/fieldaction/menu_build_water.png", _[[Unlike ships which travel to any port where they’re needed, ferries have fixed transport routes called waterways. A waterway is built just like a road: You click on a flag near the shore, choose ‘Build a waterway’, and select the path for the waterway just as you would do for a road.]]) ..
      p(_[[Waterways can be built on the open sea as well as close to the shore where the water is too shallow for big ships. The only rule is that the two triangles directly adjacent to each segment of the waterway have to be water, and the entire path needs to be within your territory.]])
   ),
   h = 300
}
ferry_3 = {
   position = "topright",
   title = _"There is more to it…",
   body = (
      h1(_"Rowboats") ..
      p(_[[Also note that waterways, unlike roads, may not be built quite as long as you want them to. Every map has a length restriction for waterways. On this map, this is 5 tiles.]]) ..
      p(_[[Always keep in mind that ferries do not transport workers. You can use them to speed up ware transport, but they can never entirely replace roads.]])
   ),
   h = 200
}
ferry_4 = {
   position = "topright",
   title = _"There is more to it…",
   body = (
      h1(_"Waterways") ..
      p(_[[Allow me to show you how to build a waterway…]])
   ),
   h = 150
}
ferry_5 = {
   position = "topright",
   title = _"There is more to it…",
   body = (
      h1(_"Ferry Yards") ..
      p(_[[Ferries are built in ferry yards. A ferry yard is a medium-sized building that needs to be built close to the shore. A ferry costs two logs and one piece of cloth and is quickly constructed. When you build a waterway, the closest idle ferry will assign itself to it.]]) ..
      p(_[[Now it’s your turn: Build a gold mine on the mountain in the south and connect it to the main land by building waterways over the streams.]])
   ),
   h = 200,
   obj_name = "build_ferries",
   obj_title = _"Build Waterways and Ferries",
   obj_body = (
      h1(_"Build waterways") ..
      p(_[[Build a gold mine on the mountain in the south, and connect it to the mainland by building waterways over the streams.]]) ..
      li_arrow(_[[A waterway is built just like a road: You click on a flag near the shore, choose ‘Build a waterway’, and select the path for the waterway just as you would do for a road.]]) ..
      li_arrow(_[[The only rule for waterway placement is that the two triangles directly adjacent to each segment of the waterway have to be water, and the entire path needs to be within your territory.]]) ..
      h1(_"Build a ferry yard") ..
      p(_[[Ferries are constructed by a ferry yard.]]) ..
      li_arrow(_[[You need to build the ferry yard close to the shore, otherwise it won’t be able to build ferries there.]])
   ),
}
ferry_6 = {
   position = "topright",
   title = _"Conclusion",
   body = (
      h1(_"About Ferries") ..
      p(_[[Now there are only a few more things I would like you to keep in mind whenever using ferries.]]) ..
      p(_[[Do remember to stop your ferry yard when you don’t need any new ferries to be produced. As long as a ferry services a waterway, its lifetime is unlimited, but unemployed ferries will rot away and eventually sink.]]) ..
      p(_[[Never forget that these rowboats are too small to carry workers. You can use them as shortcuts for ware transport within parts of your territory, but not to claim regions you can reach neither by road nor by ship. Never cut all your roads between two places connected by waterways unless you are certain no worker will ever have to walk there.]]) ..
      p(_[[And if your waterways present a significant shortcut between your road networks, your economy will tend to send many wares over the waterway. As a waterway can hold only one ferry – unlike roads, which receive a second carrier if they are very busy –, and as they often tend to be rather longer than the two or three fields recommended for roads, there is always a risk of waterways becoming severe bottlenecks in your economy. Try to build several of them in parallel to distribute the strain. If this is not possible, it is in some cases even more efficient not to use waterways.]])
   ),
   h = 450
}
ferry_7 = {
   position = "topright",
   title = _"Congratulations",
   body = (
      h1(_"Congratulations") ..
      p(_[[You have learnt all there is to learn about ships and ferries!]]) ..
      p(_[[There is also another island where you can build a port if you wish to try launching another expedition.]]) ..
      p(_[[As always, you can continue playing and practice a bit more on this map if you like, or get your captain’s license ready and try out your new skills in a real game…]])
   ),
   h = 150
}
