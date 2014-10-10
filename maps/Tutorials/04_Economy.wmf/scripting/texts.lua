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

intro1 = {
   title = _"Economy and its settings",
   body = rt(
      h1(_[[Economy]]) ..
      p(_[[Welcome back. In this tutorial, I'll tell you what you can do to check how well your economy works.]]) ..
      p(_[[Building your economy up and making it work well and grow is the main part of Widelands. But you can't control the workers directly - they follow the frame conditions you set.]]) ..
      p(_[[That is what I'll show you in this tutorial: What action can you take to define those frame conditions?]])
   )
}

intro2 = {
   position = "topright",
   field = wl.Game().map:get_field(67,70), -- somewhere near the border
   title = _"A peaceful land",
   body = rt(
      p(_[[Now about the map: You settled in a nice valley between two mountains, rich in marble, iron ore and coal. All were hoping for a peaceful life.]]) ..
      p(_[[But one day, you discovered a barren, waste land with abandoned buildings in the east. A strange aura came from there, and no one wanted to set a foot there. But the border could not be let undefended, and so you have three castles constructed.]]) ..
      p(_[[You you have not been prepared for a war, and so you have to hurry to build up an army.]])
   )
}

tavern_burnt_down = {
   position = "topright",
   title = _"The tavern is burning!",
   body = rt(
      h1(_[[An accident]]) ..
      p(_[[Oh no, look at this: Our tavern is burning! In all the hurry, our innkeeper accidentally dropped a torch. She is fine, but we could not extinguish the fire in time.]])
   )
}

building_stat = {
   position = "topright",
   title = _"Building statistics",
   body = rt(
      h1(_[[Check for your taverns]]) ..
      p(_[[At first, we should check how many taverns we currently have. Widelands offers you a list where you can easily check this.]]) ..
      p(_[[At first, you have to open the statistics menu (the second button from the left at the bottom of your screen). We will need this menu several times. Afterwards, choose the 'Building statistics'.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Open the building statistic menu.]])
   ),
   obj_name = "open_building_stat",
   obj_title = _"Open the building statistics window.",
   obj_body = rt(
      p(_[[The building statistics window gives you an overview over the buildings you have.]]) ..
      paragraphdivider() ..
      --TRANSLATORS: "it" refers to the building statistic window
      listitem_bullet(_[[Open it. You can access it from the statistic window.]]) ..
      listitem_arrow(_[[The statistic window is accessible vie the second button at the bottom of your screen. It provides several menus that give you information about the game.]])
   )
}

inventory1 = {
   position = "topright",
   title = _"Stock",
   body = rt(
      h1(_[[Check for your rations]]) ..
      p(_[[Ok. In the list, you've seen that you have no more taverns or inns. That means that you're not producing any rations. But let's see what we've still in stock.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Click on the 'Stock' button.]]) ..
      listitem_arrow(_[[You can also use the hotkey 'i' (like inventory) to accesss this menu quickly.]])
   ),
   obj_name = "open_inventory",
   obj_title = _"Open your stock window.",
   obj_body = rt(
      p(_[[The stock window gives you an overview over the wares you have.]]) ..
      paragraphdivider() ..
      --TRANSLATORS: "it" refers to the stock window
      listitem_bullet(_[[Open it. You can access it from the statistic window.]]) ..
      listitem_arrow(_[[The statistic window is accessible via the second button at the bottom of your screen. It provides several menus that give you information about the game.]])
   )
}

inventory2 = {
   position = "topright",
   title = _"Stock",
   body = rt(
      p(_[[The stock window has four tabs. The first (and currently selected) one shows you all the wares you have, including those on the roads, at the flags and inside the buildings waiting for processing.]]) ..
      p(_[[The second tab shows you all your workers, again those on the roads and in the buidlings summed up.]]) ..
      p(_[[The third tab shows you the wares that are stored in your headquarters, your warehouses and ports. They are not needed anywhere and are therefore your reserve.]]) ..
      p(_[[The fourth tab shows the same thing for workers.]]) ..
      p(_[[Now let's have a look at our rations: There are currently five ones on the way - that is not much for such a big economy.]]) ..
      p(_[[When you choose the third tab, you'll see that there aren't any left in your headquarters - that is not good!]])
   )
}

build_taverns = {
   position = "topright",
   title = _"New taverns",
   body = rt(
      h1(_[[We need new taverns]]) ..
      p(_[[Now that you have an overview, you should act. I think we should build more than a tavern - two or three are better. Remember: As long as we don't produce rations, our miners don't dig for ores. And without iron, we cannot forge a single helm.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build at least two taverns.]])
   ),
   obj_name = "build_taverns",
   obj_title = _"Build new taverns.",
   obj_body = rt(
      p(_[[To make our mines work, we need rations again - the more, the better.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Build at least two taverns.]])
   )
}

ware_encyclopedia = {
   title = _"Ware Encyclopedia",
   body = rt(
      p(_[[I am not sure if you could follow my remarks. Why do we need rations to get soldiers?]]) ..
      p(_[[When you've played a lot, you know such things by heart. But when you're unsure what this tribe needs for a special ware, you can easily look it up in your tribe's ware encyclopedia.]]) ..
      p(_[[This encyclopedia can be accessed via the question mark button at the bottom of your screen. For all wares of your tribe, it shows a short help text, the building that produces the ware and the needed wares.]]) ..
      p(_[[If you want, you can try it out. A soldier needs a wood lance and a helm - from there on, you can search backwards.]])
      -- TODO(codereview): Is there a place where the player can see the needed weapons?
   )
}

building_priority_settings = {
   pos = "topright",
   title = _"Priority settings",
   body = rt(
      h1(_[[Send the wares where they're needed]]) ..
      p(_[[Great. Our taverns are now built up and are supplying us with rations.]]) ..
      p(_[[At the moment, all mines are supplied with rations. If you want to prioritize a special mine, you simply have to open its menu. In the wares tab, behind every ware, you see the 'traffic lights'.]]) ..
      p(_[[When you click on the red part (low priority), the corresponding ware gets delivered less frequently. Green means that as many wares as possible should de delivered to this building, maybe because it produces something important.]]) ..
      p(_[[Currently, you probably want the output of all mines, so there is no need to set a special priority. But maybe in a real game, you could give your toolsmith a priority for iron. This makes the tool production faster, but slows down the helm production. There might be situations where this comes in handy.]])
   )
}

ware_stat = {
   pos = "topright",
   title = _"Ware Statistic",
   body = rt(
      p(_[[In the statistic menu, there is also the button 'Ware statistics'. It opens a window where you can select wares to see how their production or consumption has changed over the time.]]) ..
      p(_[[But you can also see the difference, called 'economy health'. You can see at one glance whether the production or consumption of this ware is higher, that means whether the amount increases or decreases.]]) ..
      p(_[[Finally, you can also see your absolute stock. The last two tabs are good indicators to see whether shortages are about to come. Don't forget to check them regularily!]]) ..
      p(_[[Since this game does not yet last long, you won't see much in those graphs.]])
   )
}

economy_settings1 = {
   pos = "topright",
   title = _"Economy options",
   body = rt(
      p(_[[I've shown you our stock window, where you could see which wares are at the warehouses. You remember?]]) ..
      p(_[[Now I'll tell you how you can change this setting. The menu can be accessed via any flag and is called 'Configure economy'.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Open this menu.]])
   ),
   obj_name = "open_economy_settings",
   obj_title = _"Open the 'Configure economy' menu.",
   obj_body = rt(
      paragraphdivider() ..
      listitem_bullet(_[[Open the 'Configure economy' menu.]]) ..
      listitem_arrow(_[[The menu can be accessed by clicking on any flag you own.]])
   )
}

economy_settings2 = {
   pos = "topright",
   title = _"Economy options",
   body = rt(
      p(_[[This window looks similar to the stock window, but it has additional buttons at the bottom.]]) ..
      -- TODO(codereview): What is the correct name for drag selection?
      p(_[[You first have to select one or more wares (you can also use a drag selection). Then, you can set the desired target quantity of this ware/these wares.]]) ..
      p(_[[Most buildings will only produce when your stock level (what is in your warehouses) is below the target quantity, so you indicate the reserve you want to have.]]) ..
      p(_[[An example: The default value for scythes is 1. If you build a farm, a carrier takes a scythe and becomes a farmer. Now there are no scythes left, but the target quantity is 1, therefore your toolsmith starts to produce another one.]]) ..
      p(_[[If you build two farms, only one can immediately start working. The second farm waits for its worker, who needs a scythe. If you had set the target quantity to 2 before, two scythes were available and both farms could start working right away.]]) ..
      p(_[[By changing this value, you can therefore decide which wares/tools your resources (in this case: iron) should be turned into or whether you would like to save your iron and wait what you will need it for.]]) ..
      p(_[[Only buildings that consume wares take care of this setting. Buildings that produce wares for free (e.g. your farms or wells) will always work.]]) ..
      p(_[[Now let's try it out: The current target quantity for marble columns is 10. Higher it to be prepared if you have to build up your fortification quickly.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Set the target quantity for marble columns to 20 and wait for your stonemason to produce them.]])
   ),
   obj_name = "produce_marble_columns",
   obj_title = "Produce 20 marble columns.",
   obj_body = rt(
      p(_[[Sometimes, you need many wares of the same time quickly - faster than they can be produced. In this case, it is good to have enough on reserve.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[To be prepared for additional fortification, you should produce 20 marble columns.]]) ..
      listitem_arrow(_[[Your stonemason will not produce marble columns when they are not needed. You have to increase the target quantity.]]) ..
      listitem_arrow(_[[To do so, click on any flag and choose 'Configure economy'. In this menu, you can decide how many wares of each type you wish to have in stock.]])
   )
}

warehouse_preference_settings = {
   pos = "topright",
   title = _"Warehouse preferences",
   body = rt(
      h1(_[[Bring the marble columns to the front line]]) ..
      p(_[[It is great that we are producing marble columns, but it would be great if they were stored where we need them.]]) ..
      p(_[[Normally, produced wares are brought to the closest warehouse if they are not needed elsewhere. In this case, this means in our headquarters. But we would like to have them in the warehouse near our fortresses.]]) ..
      p(_[[Every warehouse has four buttons to set the preference. They are self-explanatory and allow you to store wares here or not.]]) ..
      paragraphdivider() ..
      listitem_bullet(_[[Bring all the marble columns to the warehouse near the front line.]]) ..
      -- TODO(codereview): Is this how the "Preferably store here" option works? (change the text below, too)
      listitem_arrow(_[[To achieve this, you have to do two things: Set a preference for the marble columns in the desired warehouse. All marble columns produced in the future will be brought there.]]) ..
      listitem_arrow(_[[To get the marble columns out of your headquarters, you have to click on the remove button there.]])
   ),
   obj_name = "bring_marble_columns_to_front",
   obj_title = _"Bring 20 marble columns to the front line.",
   obj_body = economy_settings2.obj_body .. rt(
      p(_[[To decide where your wares get stored, you can use the preference buttons in the warehouses.]]) ..
      listitem_bullet(_[[Bring all the marble columns to the warehouse near the front line.]]) ..
      listitem_arrow(_[[To achieve this, you have to do two things: Set a preference for the marble columns in the desired warehouse. All marble columns produced in the future will be brought there.]]) ..
      listitem_arrow(_[[To get the marble columns out of your headquarters, you have to click on the remove button there.]])
   )
}

conclusion = {
   title = _"Borders secured",
   body = rt(
      h1(_[[We're safe now]]) ..
      p(_[[Great. We now have enough marble columns so that in case of an aggressor, we can build up our fortification. But I do not think that that'll be necessary. So far, no enemy has shown up.]]) ..
      p(_[[I hope I could teach you how you can control the economy in Widelands. There are many options and they are surely confusing. Even if you've only understood a few concepts, you mustn't give up. Try them out in some games, become familiar with them and experience the possibilities. Then, return to this tutorial and learn the rest!]]) ..
      p([[]]) ..
      p(_[[This was the last tutorial I had prepared for you. I've now taught you everything I know. There are still secrets hidden in this world even I don't know. I will now search a quiet place to spend my sunset years. If you have still questions, the Widelands community will surely help you. You can find it under:]])
   ) ..
   rt("text-align=center", "<p font-size=24 font-decoration=underline>http://www.widelands.org</p>")
}