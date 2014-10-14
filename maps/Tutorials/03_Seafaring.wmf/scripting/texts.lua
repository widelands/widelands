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
   pos = "topright",
   field = wl.Game().map.player_slots[1].starting_field,
   title = _"Seafaring",
   body = rt(
      h1(_"Seafaring Tutorial") ..
      p(_[[Welcome back. In this tutorial, you are going to learn the most important things about seafaring, that is ships, ports, and expedition.]]) ..
      p([[]]) .. -- empty line
      p(_[[But let me first give you an overview about your territory: Here in the south, you have a whole economy with almost everything you need.]])
   ),
}

intro_north = {
   pos = "topright",
   field = wl.Game().map:get_field(36,20), -- field of the castle
   title = _"The Northern Part",
   body = rt(
      p(_[[Here in the northern part, you only have a goldmine and a warehouse. While the miners are supplied well with food, there is no way to bring the goldores to our smelting works in the southern part.]]) ..
      p(_[[We have tried to build a road, but the mountain is too wide and too steep. We have therefore only one possibility: We need to establish a ship route between those two parts.]]) ..
      p(_[[But I don't want to rush you: You have just arrived here and you would probably like to have a closer look at your camp. I will also take a short break and be back soon.]])
   ),
}

tell_about_port = {
   pos = "topright",
   title = _"Ports",
   body = rt(
      h1(_"Ports") ..
      p(_[[For everything you do on the high seas, you need a port at the shore. Ports are like headquarters: They can store wares, workers and soldiers. The soldiers inside will automatically come out when an enemy attacks the port.]]) ..
      p(_[[But ports offer the possibility to transport wares via ships. When you click on the port you already have, you will notice two additional tabs: Wares and workers in the dock. They are waiting for a ship to transport them to another port. Currently, there are none because we have not yet built a second port. So let's change this!]])
   ),
}

tell_about_port_building = {
   pos = "topright",
   field = second_port_field, --wl.Game().map:get_field(37,27), -- field of the port NOCOM
   title = _"Building ports",
   body = rt(
      h1(_"How to build a port?") ..
      p(_[[Ports are big buildings, but they can only be built at special locations: Those marked with the]])
   ) ..
   rt("image=pics/port.png", p(_[[blue port space icon.]])) ..
   rt(
      p(_[[Port spaces are set by the map designer, so a map either contains them or not. They might, however, be hidden under trees or be blocked by surrounding buildings.]]) ..
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
      listitem_arrow(_[[Although ports act as warehouses, you should not build more than necessary: They cost quartz, diamonds and gold, which makes them quite expensive.]])
   ),
}

tell_about_shipyard = {
   pos = "topright",
   title = _"Constructing ships",
   body = rt(
      h1(_"Let's build ships") ..
      p(_[[Great. Your port has just been finished. Now we have to take care of ships.]]) ..
      p(_[[Ships are constructed in a shipyard by a shipwright. We have to build one somewhere close to the shore. You should do it in your main camp because only this provides the needed materials.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build a shipyard at the cost. It is a medium building.]])
   ),
   obj_name = "build_shipyard",
   obj_title = _"Build a shipyard",
   obj_body = rt(
      h1(_"Build a shipyard") ..
      p(_[[Ships are produced in a shipyard.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build a shipyard close to the shore of the southern part of your territory.]]) ..
      listitem_arrow(_[[The shipyard is a medium building. Although it can be built everywhere on the map, the shipwright only works when he is close to the water and there are not trees or roads at the coast.]])
   ),
}

tell_about_ships = {
   pos = "topright",
   title = _"Constructing ships",
   body = rt(
      h1(_"Waiting for the ships") ..
      p(_[[Very good. Your shipyard is finished and your shipwright immediately starts working. For the construction of ships, he needs logs, planks and spidercloth, which will be transported to the shipyard. The shipwright takes the ware he needs to a free spot at the coast and builds a ships there. When the first ship is finished, it will drive onto the sea and the shipwright will construct another one.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[We should wait until we have two ships. That should be enough for now.]]) ..
      listitem_arrow(_[[You need to stop your shipyard when you have enough ships. Otherwise, your shipwright will consume all your logs and spidercloth, producing dozens of ships.]])
   ) ..
   rt("image=pics/stop.png",p(_[[This is the icon. You find it in the building window.]])),
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
   pos = "topright",
   title = _"No Iron",
   body = rt(
      h1(_"We lack iron") ..
      p(_[[Your second ship might not finished yet, but we have an urgent problem.]]) ..
      p(_[[As you surely have already noticed, there is no iron in the mountain in the west. We have plenty of coal and goldores, but without ironores, we cannot produce any tools.]]) ..
      p(_[[Although it might take long and is expensive and not without dangers (who knows what monsters live in the sea?), I see no other possibility: We have to make an expedition into the unknown seas.]])
   )
}

expedition2 = {
   pos = "topright",
   -- TRANSLATORS: This shall be the beginning of a poem
   title = _"A trip by the sea what fun it can be",
   body = rt(
      h1(_"Expeditions") ..
      p(_[[In an expedition, you send a ship out to discover new islands and maybe found a colony there.]])
   ) ..
   rt("image=pics/start_expedition.png",p(_[[Expeditions can be started in every port. Then, all needed wares are transported to that port. The wares are exactly those your tribe needs to build a port (you build a port far away from home, so that is not surprising), and of course you need a builder, too. When everything is prepared, a ship will come and pick it up. You can check out the missing wares in the fifth tab of your port (it will appear when you've started an expedition).]]) ..
      p(_[[Now try this out. I will tell you later what are the next steps.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Start an expedition in any of your ports.]])
   ),
   obj_name = "start_expedition",
   obj_title = _"Start an expedition",
   obj_body = rt(
      paragraphdivider() ..
      listitem_bullet(_[[Start an expedition.]])
   ) ..
   rt("image=pics/start_expedition.png",p(_[[To do so, click on the 'Start Expedition' button in any port. A new tab where you can see the needed wares will appear.]])
   )
}

expedition3 = {
   pos = "topright",
   title = _"Off to new pastures",
   body = rt(
      h1(_"The ship is ready") ..
      p(_[[Your expedition should be ready about now. (If not, please wait some more time. You will receive a message.) Your ship is waiting for your orders in front of your port. It does not transport wares anymore. The control is self-explanatory: You can send your ship in any of the six main directions of the Widelands map. When it has reached a coast, you can make it travel around the coast, where it looks for suitable places for landing. With the button in the centre of the ship's control window, you can construct a port.]]) ..
      p(_[[The wares will then be unloaded and the ship takes the task of transporting wares again. The builder will start his work and build a port.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Search for an island with a mountain, and look for a port space there. Colonize the island.]])
   ),
   obj_name = "found_settlement",
   obj_title = _"Found a settlement",
   obj_body = rt(
      paragraphdivider() ..
      listitem_bullet(_[[Navigate your ship to an island that could contain ironore.]]) ..
      listitem_arrow(_[[When you click on the expedition ship, a window opens where you can control your ship.]]) ..
      listitem_bullet(_[[When you have found a suitable port space, build a port there.]])
   )
   
   
}

conclusion = {
   pos = "topright",
   field = port_on_island, --wl.Game().map:get_field(102, 36), -- port on island NOCOM
   title = _"Conclusion",
   body = rt(
      h1(_"Congratulations") ..
      p(_[[You've lead the expedition to a successful end and founded a new colony. I've sent out some geologists - they already report that they've found some ironore.]]) ..
      p(_[[In this scenario, you've learned everything about seafaring: how to build ports and ships and how to send out an expedition. Remember that expeditions are sometimes the fastest way to reach essentially needed resources - and sometimes the only one.]]) ..
      p(_[[But I want to speak a word of warning. Ports are like headquarters: They can be attacked by a nearby enemy. While your headquarters has soldiers to defend, your newly built port hasn't. You should therefore avoid to settle next to an enemy.]]) ..
      p(_[[On this map, there is no enemy to fear. As always, you can continue playing and watch how the ships deliver wares to the island when you construct some buildings here. There is also another island where you can build a port.]])
   )
}
