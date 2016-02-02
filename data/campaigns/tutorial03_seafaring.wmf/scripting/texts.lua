-- =======================================================================
--                      Texts for the tutorial mission
-- =======================================================================

-- =========================
-- Some formating functions
-- =========================

include "scripting/formatting.lua"
include "scripting/format_scenario.lua"


-- =============
-- Texts below
-- =============

intro_south = {
   position = "topright",
   field = sf,
   title = _"Seafaring",
   body = rt(
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
   body = rt(
      p(_[[Here in the northern part, you only have a goldmine and a warehouse. While the miners are supplied well with food, there is no way to transport the gold ore to our smelting works in the southern part.]]) ..
      p(_[[We have tried to build a road, but the mountain is too wide and too steep. We therefore have only one possibility: we need to establish a sea lane between these two parts.]]) ..
      p(_[[But I don’t want to rush you: you have just arrived here and you would probably like to have a closer look at your camp. I will also take a short break and be back soon.]])
   ),
   h = 350
}

tell_about_port = {
   position = "topright",
   title = _"Ports",
   body = rt(
      h1(_"Ports") ..
      p(_[[For everything you do on the high seas, you need a port at the shore. Ports are like headquarters: they can store wares, workers and soldiers. The soldiers inside will automatically come out when an enemy attacks the port.]]) ..
      p(_[[Additionally, ports offer the possibility of transporting wares via ships. When you click on the port you already have, you will notice two additional tabs: wares and workers in the dock. They are waiting for a ship to transport them to another port. Currently, there are none because we have not yet built a second port. So let’s change this!]])
   ),
   h = 350
}

tell_about_port_building = {
   position = "topright",
   field = second_port_field,
   title = _"Building ports",
   body = rt(
      h1(_"How to build a port") ..
      p(_[[Ports are big buildings, but they can only be built at special locations: those marked with the]])
   ) ..
   rt("image=images/wui/overlays/port.png", p(_[[blue port space icon.]])) ..
   rt(
      p(_[[Port spaces are set by the map designer, so a map will either contain them or not. They might, however, be hidden under trees or be blocked by surrounding buildings.]]) ..
      p(_[[You might already have noticed that you have such an icon next to your castle.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build a port in the northern part of your camp.]])
   ),
   obj_name = "build_port",
   obj_title = _"Build a port in the northern part of your camp.",
   obj_body = rt(
      h1(_"Build a port") ..
      p(_[[You always need a port when you want to transport wares with a ship.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build a port next to your castle, on the blue port space icon.]]) ..
      listitem_arrow(_[[Ports are built like normal buildings, but are only available on blue port spaces. Just click on a field with the icon and the building menu automatically offers you to build a port.]]) ..
      listitem_arrow(_[[Although ports act as warehouses, you should not build more than necessary: they cost quartz, diamonds and gold, which makes them quite expensive.]])
   ),
}

tell_about_shipyard = {
   position = "topright",
   title = _"Constructing ships",
   body = rt(
      h1(_"Let’s build ships") ..
      p(_[[Great. Your port has just been finished. Now we need some ships.]]) ..
      p(_[[Ships are constructed in a shipyard by a shipwright. We have to build one somewhere close to the shore.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build a shipyard close to the coast. It is a medium building.]])
   ),
   h = 300,
   obj_name = "build_shipyard",
   obj_title = _"Build a shipyard",
   obj_body = rt(
      h1(_"Build a shipyard") ..
      p(_[[Ships are produced in a shipyard.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build a shipyard close to the shore of the southern part of your territory.]]) ..
      listitem_arrow(_[[The shipyard is a medium building. Although it can be built everywhere on the map, the shipwright only works when he is close to the water and there are no trees or roads at the shoreline.]])
   ),
}

tell_about_ships = {
   position = "topright",
   title = _"Constructing ships",
   body = rt(
      h1(_"Waiting for the ships") ..
      p(_[[Very good. Your shipyard is finished and your shipwright immediately started working. For the construction of ships, he needs logs, planks and spidercloth, which will be transported to the shipyard.]] .. " " ..
      _[[The shipwright will take the ware he needs to a free spot at the shoreline and build a ship there. When the first ship is finished, it will launch onto the sea, and the shipwright will construct another one.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[We should wait until we have two ships. That should be enough for now.]]) ..
      listitem_arrow(_[[You need to stop your shipyard when you have enough ships. Otherwise, your shipwright will consume all your logs and spidercloth, producing dozens of ships.]])
   ) ..
   rt("image=images/ui_basic/stop.png",p(_[[This is the icon for stopping production. You will find it in the building window.]])),
   obj_name = "wait_for_ships",
   obj_title = _"Construct two ships",
   obj_body = rt(
      p(_[[Ships are constructed automatically when the shipyard is complete and the needed wares have been delivered.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Wait until the shipwright has constructed two ships.]]) ..
      listitem_arrow(_[[Do not forget to stop your shipyard when you have enough ships.]])
   )
}

expedition1 = {
   position = "topright",
   title = _"No Iron",
   body = rt(
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
   body = rt(
      h1(_"Expeditions") ..
      p(_[[During an expedition, you send a ship out to discover new islands and maybe found a colony there.]])
   ) ..
   rt("image=images/wui/buildings/start_expedition.png",p(_[[Expeditions can be started in every port. Then, all needed wares are transported to that port. The wares are exactly those your tribe needs to build a port (your goal is to build a port far away from home, so this is not surprising), and of course you need a builder, too. When everything is prepared, a ship will come and pick it up.]] .. " " ..
      _[[You can check out the needed wares in the fifth tab of your port (it will appear when you’ve started an expedition).]]) ..
      p(_[[Now try this out. I will tell you later what the next steps are.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Start an expedition in any of your ports.]])
   ),
   obj_name = "start_expedition",
   obj_title = _"Start an expedition",
   obj_body = rt(
      paragraphdivider() ..
      listitem_bullet(_[[Start an expedition.]])
   ) ..
   rt("image=images/wui/buildings/start_expedition.png",p(_[[To do so, click on the ‘Start Expedition’ button in any port. A new tab where you can see the needed wares will appear.]])
   )
}

expedition3 = {
   position = "topright",
   title = _"Off to greener pastures",
   body = rt(
      h1(_"Start your expedition") ..
      p(_[[Your expedition ship is ready. It is waiting for your orders in front of your port. It isn’t transporting wares anymore. Use its buttons to send your ship in any of the six main directions of the Widelands map. When it has reached a coastline, you can make it travel around the coast, where it will look for suitable places for landing.]] .. " " ..
      _[[Once a port space has been found, you can construct a new port with the button in the center of the ship’s control window.]]) ..
      p(_[[The wares will then be unloaded, and the ship will take up the task of transporting wares once again. The builder will start his work and build a port.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Search for an island with a mountain, and look for a port space there. Colonize the island.]])
   ),
   obj_name = "found_settlement",
   obj_title = _"Found a settlement",
   obj_body = rt(
      paragraphdivider() ..
      listitem_bullet(_[[Navigate your ship to an island that could contain iron ore.]]) ..
      listitem_arrow(_[[When you click on the expedition ship, a window opens where you can control your ship.]]) ..
      listitem_bullet(_[[When you have found a suitable port space, build a port there.]])
   )
}

conclusion = {
   position = "topright",
   field = port_on_island,
   title = _"Conclusion",
   body = rt(
      h1(_"Congratulations") ..
      p(_[[You’ve lead the expedition to a successful end and founded a new colony. I’ve sent out some geologists – they already report that they’ve found some iron ore.]]) ..
      p(_[[In this scenario, you’ve learned everything about seafaring: how to build ports and ships and how to send out an expedition. Remember that expeditions are sometimes the fastest way to reach essential resources – and sometimes the only one.]]) ..
      p(_[[But I want to speak a word of warning. Ports are like headquarters: they can be attacked by a nearby enemy. While your headquarters has soldiers to defend it, your newly built port won’t. You should therefore avoid settling next to an enemy.]]) ..
      p(_[[On this map, there is no enemy to fear. As always, you can continue playing and watch how the ships deliver wares to the island when you construct some buildings there. There is also another island where you can build a port.]])
   ),
   h = 450
}
