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

intro1 = {
   title = _"Your Economy and its Settings",
   body = (
      h1(_[[Economy]]) ..
      p(_[[Welcome back. In this tutorial, I’ll tell you what you can do to check how well your economy works.]]) ..
      p(_[[Building your economy up and making it work well and grow is the main part of Widelands. But you can’t control the workers directly – they will follow the general conditions you set.]]) ..
      p(_[[This is what I’ll show you in this tutorial: what actions can you take to define those general conditions?]])
   ),
   h = 300
}

intro2 = {
   position = "topright",
   field = field_near_border,
   title = _"A Peaceful Land",
   body = (
      p(_[[Now about the map: you have settled in a nice valley between two mountains, rich in marble, iron ore and coal. All were hoping for a peaceful life.]]) ..
      p(_[[But one day, you discovered a barren wasteland with abandoned buildings in the east. A strange aura came from there, and no one wanted to set foot there. But the border could not be left undefended, and so you constructed three castles.]]) ..
      p(_[[You had not been prepared for war, and you have to hurry now to build up an army.]])
   ),
   h = 300
}

tavern_burnt_down = {
   position = "topright",
   title = _"The Tavern is Burning!",
   body = (
      h1(_[[An accident]]) ..
      p(_[[Oh no, look at this: our tavern is burning! In all the hurry, our innkeeper accidentally dropped a torch. She is fine, but we could not extinguish the fire in time.]])
   ),
   w = 300,
   h = 250
}

building_stat = {
   position = "topright",
   title = _"Building statistics",
   body = (
      h1(_[[Check out your taverns]]) ..
      p(_[[At first, we should find out how many taverns we currently have. Widelands offers you a window where you can easily check this.]]) ..
      li_image("images/wui/menus/menu_toggle_menu.png", _[[First, you will have to open the statistics menu (you can find the corresponding button at the bottom). We will need this menu several times.]]) ..
      li_image("images/wui/menus/menu_building_stats.png", _[[Afterwards, choose the ‘Building statistics’.]]) ..
      li(_[[Open the building statistics window.]]) ..
      li_arrow(_[[You can also use the hotkey ‘b’.]])
   ),
   h = 350,
   obj_name = "open_building_stat",
   obj_title = _"Open the building statistics window",
   obj_body =
      li_image("images/wui/menus/menu_building_stats.png", _[[The building statistics window gives you an overview over the buildings you have.]]) ..
      -- TRANSLATORS: "it" refers to the building statistics window
      li(_[[Open it. You can access it from the statistics menu.]]) ..
      li_arrow(_[[The statistics menu is accessed via the second button at the bottom. It provides several windows that give you information about the game.]])
}

explain_building_stat = {
   title = _"Building Statistics",
   body = (
      p(_[[This is the building statistics window. It shows you all buildings you can own, sorted by their size.]]) ..
      p(_[[Let me now explain what all those numbers mean:]]) ..
      li(_[[‘2/1’ below the quarry: This means that you have two quarries, plus another one which is under construction.]]) ..
      li(_[[‘0%’: This indicates the average productivity of all buildings of that type. You have just started this game, therefore none of your buildings has done any work yet, but they are going to start working soon.]]) ..
      li(_[[‘2/4’ below your sentry: For military buildings, the stationed soldiers are shown instead of a productivity. You want to have four soldiers in your sentries, but only two soldiers are stationed in this kind of building. This leaves two vacant positions – we really need more soldiers.]]) ..
      li_arrow(_[[In both cases, the color (green - yellow - red) signals you how good the value is.]]) ..
      li(_[[If you click on a building, you can scroll through the buildings of the selected type.]]) ..
      li(_[[If you don’t have any building of a particular building type, it will be shown greyed out.]]) ..
      h2(_[[Now it’s your turn]]) ..
      p(_[[This is enough explanation for now. Now try it out yourself. We want to know whether we still have taverns, so you have to choose the ‘Medium buildings’ tab. Close the building statistics menu afterwards.]])
   ),
   obj_name = "check_taverns",
   obj_title = _"Look up how many taverns you have",
   obj_body = (
      li(_[[Choose the ‘Medium buildings’ tab in the building statistics window.]]) ..
      li(_[[Look up how many taverns you have.]]) ..
      li_arrow(_[[Below every building, there are two lines. The first one shows the number of buildings you own and how many are under construction. The second line shows the average productivity if it is a production site or training site, or the stationed and desired soldiers in military buildings.]]) ..
      li(_[[Close the building statistics window when you are done.]])
   )
}

reopen_building_stat = {
   title = _"You closed the building statistics window!",
   body = (
      p(_[[You have closed the building statistics window. I didn’t notice that you switched to the medium buildings to look up the number of taverns. Would you please be so nice and show it to me?]])
   ),
   show_instantly = true,
   w = 300,
   h = 250
}

reopen_building_stat_obj = {
   obj_name = "open_building_stat_again",
   obj_title = _"Open the building statistics window again",
   obj_body = (
      p(_[[You closed the building statistics window, although you have not yet looked up the number of taverns.]]) ..
      -- TRANSLATORS: "it" refers to the building statistics window.
      li(_[[Please reopen it and choose the second tab (medium buildings).]])
   ),
   h = 250
}

inventory1 = {
   position = "topright",
   title = _"Stock",
   body = (
      h1(_[[Check for rations]]) ..
      p(_[[OK. In the list, you’ve seen that you have no more taverns or inns. That means that you’re not producing any rations. But let’s see what we still have in stock.]]) ..
      li_image("images/wui/menus/menu_stock.png", _[[Click on the ‘Stock’ button.]]) ..
      li_arrow(_[[You can also use the hotkey ‘i’ (as in ‘inventory’) to access this window quickly.]])
   ),
   h = 300,
   obj_name = "open_inventory",
   obj_title = _"Open your stock window",
   obj_body = (
      p(_[[The stock menu window gives you an overview over the wares you currently have.]]) ..
      -- TRANSLATORS: "it" refers to the stock menu window
      li(_[[Open it. You can access it from the statistics menu.]]) ..
      li_arrow(_[[The statistics menu is accessed via the second button at the bottom. It provides several windows that give you information about the game.]])
   )
}

inventory2 = {
   title = _"Stock",
   body = (
      p(_[[The stock menu window has four tabs. The first (and currently selected) one shows you all your current wares, including those on roads, at flags and inside buildings waiting for processing.]]) ..
      p(_[[Looking at the rations, there are currently only five in total, probably on their way to somewhere. Five rations are not much for such a big economy.]]) ..
      p(_[[The second tab shows you all your workers, again those on roads and in buildings summed up.]]) ..
      p(_[[Now have a look at these two tabs. When you click on the]]) ..
      li_image("images/wui/stats/menu_tab_wares_warehouse.png", _[[third tab (‘Wares in warehouses’), I’ll continue.]])
   ),
   h = 350,
   show_instantly = true,
   obj_name = "switch_stock_tab",
   obj_title = _"Switch to the third tab in the stock menu window",
   obj_body = (
      p(_[[Have a look at the first two tabs in the stock menu window. They show all the wares and workers you have.]]) ..
      li(_[[When you have seen enough, switch to the third tab.]])
   ),
}

inventory3 = {
   title = _"Stock",
   body = (
      p(_[[The third tab shows you the wares that are stored in your headquarters, your warehouses and ports. They are not needed anywhere and are therefore your reserve.]]) ..
      p(_[[The fourth tab shows the same thing for workers.]]) ..
      p(_[[The third tab tells you that there are no rations left in your headquarters – that’s not good!]])
   ),
   show_instantly = true,
   h = 300
}

reopen_stock_menu = {
   title = _"You closed the stock window!",
   body = (
      p(_[[You have closed the stock menu window, but I have not yet finished with my explanation. Would you please reopen it and choose the third tab?]])
   ),
   show_instantly = true,
   w = 300,
   h = 250
}

reopen_stock_menu_obj = {
   obj_name = "open_stock_menu_again",
   obj_title = _"Open the stock window again",
   obj_body = (
      p(_[[You closed the stock menu window before I finished telling you everything about it. If you already know everything, please feel free to leave this tutorial at any time.]]) ..
      -- TRANSLATORS: "it" refers to the stock menu window.
      li(_[[Otherwise, please reopen it and choose the third tab.]])
   ),
   h = 250
}

build_taverns = {
   position = "topright",
   title = _"New taverns",
   body = (
      h1(_[[We need new taverns]]) ..
      p(_[[Now that you have an overview, you should act. I think we should build more than one tavern – two or three are better. Remember: as long as we don’t produce rations, our miners won’t dig for ore. And without iron, we cannot forge a single helm.]]) ..
      li(_[[Build at least two taverns.]])
   ),
   h = 300,
   obj_name = "build_taverns",
   obj_title = _"Build new taverns",
   obj_body = (
      p(_[[To make our mines work, we need rations again – the more, the better.]]) ..
      li(_[[Build at least two taverns.]])
   )
}

ware_encyclopedia = {
   title = _"Encyclopedia",
   body = (
      h1(_[[How to get help]]) ..
      p(_[[Of course, it is difficult to remember all of my remarks and advice. For example, you might ask yourself: ‘Why do we need rations to get soldiers?’]]) ..
      p(_[[When you’ve played a lot, you will know all these things by heart. But until then or if you’re unsure about your tribe’s needs and abilities and how its buildings and workers operate, you can look it up easily in our tribe-specific in-game help and encyclopedia.]]) ..
      p(_[[This encyclopedia can be accessed via the help button at the bottom right.]])..
      li_image("images/ui_basic/menu_help.png", _[[Please open the in-game help, and I’ll explain its contents to you.]])
   ),
   h = 350,
   show_instantly = true,
   obj_name = "open_encyclopedia",
   obj_title = _"Open the in-game help window",
   obj_body = (
      li_image("images/ui_basic/menu_help.png", _[[The encyclopedia window contains the in-game help and an encyclopedia of the tribe you’re currently playing.]]) ..
      -- TRANSLATORS: "it" refers to the encyclopedia window
      li(_[[Open it. You can access it via the button at the bottom of the screen.]]) ..
      li_arrow(_[[Alternatively, you can access it directly with the ‘F1’ key.]])
   ),
}

explain_encyclopedia = {
   position = "topright",
   title = _"Encyclopedia details",
   body = (
      h1(_[[The Encyclopedia Categories]]) ..
      p(_[[The encyclopedia has five tabs:]])..
      div("width=100%", div("float=left padding_r=14 padding_t=5",p(img("images/logos/wl-ico-32.png"))) .. p(_[[The ‘General’ tab contains information about all the basic interface controls for Widelands. Furthermore, you can find tips that are relevant for your tribe there.]])) ..
      div("width=100%", div("float=left padding_r=18 padding_t=15 padding_b=15 padding_l=4",p(img("images/wui/buildings/menu_tab_wares.png"))) .. p(_[[The ‘Wares’ tab shows information about the wares that your tribe needs, including a short help text, a list of buildings that produce each ware, the needed wares to produce it and where the ware is consumed.]])) ..
      div("width=100%", div("float=left padding_r=16",p(img("images/wui/buildings/menu_tab_workers.png"))) .. p(_[[The ‘Workers’ tab shows information about your tribe’s workers in a similar manner to the wares in the second tab.]])) ..
      div("width=100%", div("float=left padding_r=18 padding_t=5 padding_l=4",p(img("images/wui/stats/genstats_nrbuildings.png"))) .. p(_[[The ‘Buldings’ tab contains all the necessary information about the buildings of your tribe.]])) ..
      li_image("tribes/immovables/field_harvested/idle_00.png", _[[Finally, the ‘Immovables’ tab shows information about the specific immovables that your tribe’s workers can place on the map.]]) ..
      li(_[[Now use the encyclopedia to find out how to create new soldiers.]]) ..
      li_arrow(_[[A soldier needs a wooden spear and a helmet – from there on out, you can search backwards to find the wares and the buildings you need to supply your barracks where the soldier is recruited. When you are finished, just close the encyclopedia window.]])
   ),
   h = 450,
   show_instantly = true,
   obj_name = "check_wares",
   obj_title = _"Look up which wares are needed to recruit soldiers",
   obj_body = (
      li(_[[Choose the ‘Wares’ tab in the encyclopedia window.]]) ..
      li(_[[Look up what is needed to produce a helmet and what is needed to produce a wooden spear.]]) ..
      li_arrow(_[[If you want, you may further look up what is needed to produce the wares you just looked up.]]) ..
      li(_[[Close the encyclopedia window when you are done.]])
   )
}

reopen_encyclopedia = {
   title = _"You closed the encyclopedia!",
   body = (
      p(_[[You have closed the encyclopedia window, but I didn’t notice that you were trying to find out which wares are needed to recruit a soldier. Would you please reopen it and do so?]])
   ),
   show_instantly = true,
   w = 300,
   h = 250
}

reopen_encyclopedia_obj = {
   obj_name = "open_encyclopedia_again",
   obj_title = _"Open the encyclopedia window again",
   obj_body = (
      p(_[[You closed the encyclopedia window without searching for the information we need. If you already know everything, please feel free to leave this tutorial at any time.]]) ..
      li(_[[Otherwise, please reopen the encyclopedia window and choose the second tab.]])
   ),
   h = 250
}

building_priority_settings = {
   position = "topright",
   title = _"Priority Settings",
   body = (
      h1(_[[Send the wares where they’re needed]]) ..
      p(_[[Great. Our taverns have now been built up and are supplying us with rations.]]) ..
      p(_[[At the moment, all mines are supplied with rations. If you want to prioritize a special mine, you simply have to open its window. In the wares tab, behind every ware, you can see ‘traffic lights’.]]) ..
      p(_[[When you click on the red dot (low priority), the corresponding ware gets delivered less frequently. Green means that as many wares as possible should be delivered to this building, maybe because it produces something important.]]) ..
      p(_[[In our situation, you might want to work the bakeries as fast as possible because they supply our taverns, so you could set water to the highest priority for them. The other buildings (for example the donkey farm) would then get less water, but the bakery could work faster.]])
      -- we cannot check whether the user does this, so no objective
      -- see bug https://bugs.launchpad.net/widelands/+bug/1380288
   )
}

ware_stats1 = {
   position = "top",
   title = _"Ware Statistics",
   body = (
      p(_[[In the statistics menu, there is also a]]) ..
      li_image("images/wui/menus/menu_ware_stats.png", _[[‘Ware statistics’ button.]]) ..
      -- TRANSLATORS: "it" refers to the ware statistics button
      li(_[[Click on it.]])
   ),
   w = 200,
   h = 200,
   obj_name = "open_ware_stat",
   obj_title = _"Open the ware statistics window",
   obj_body = (
      li(_[[Open the ‘Ware statistics’ window, accessed via the statistics menu.]])
   )
}

ware_stats2 = {
   title = _"Ware Statistics",
   body = (
      p(_[[In this menu window, you can select wares to see how their production or consumption has changed over time. Try it out with some wares.]]) ..
      li(_[[I’ll continue as soon as you click on the]]) ..
      li_image("images/wui/stats/menu_tab_wares_econ_health.png", _[[third tab (‘Economy health’).]])
   ),
   h = 250,
   show_instantly = true,
   obj_name = "switch_ware_stat_tab_to_third",
   obj_title = _"Switch to the third tab in the ware statistics menu window",
   obj_body = (
      p(_[[The first two tabs show you the production and consumption of any ware. You can toggle them by simply clicking on them.]]) ..
      li(_[[When you have seen enough, switch to the third tab.]])
   )
}

ware_stats3 = {
   title = _"Ware Statistics",
   body = (
      p(_[[In this tab, you can see the difference between production and consumption, called ‘economy health’. You can see at one glance which one of those two is higher for the selected ware, that means whether the amount increases or decreases.]]) ..
      p(_[[Now try this out. You can also compare it with the two previous tabs.]]) ..
      li(_[[Click on the last tab when I should continue.]])
   ),
   h = 250,
   show_instantly = true,
   obj_name = "switch_ware_stat_tab_to_forth",
   obj_title = _"Switch to the last tab in the ware statistics menu window",
   obj_body = (
      p(_[[The third tab shows you the economy health of the ware. When the value is positive, this means your stock is growing.]]) ..
      li(_[[When you have seen enough, switch to the fourth tab.]])
   )
}

ware_stats4 = {
   title = _"Ware Statistics",
   body = (
      p(_[[In the last tab, you can also see your absolute stock. It will increase when the economy health is positive, and decrease otherwise. Compare the two graphs!]]) ..
      p(_[[The last two tabs are good indicators to see whether shortages are about to come. Don’t forget to check them regularly!]]) ..
      li(_[[Close this window when you’re done.]])
   ),
   h = 250,
   show_instantly = true,
   obj_name = "close_ware_stats",
   obj_title = _"Close the ware statistics window",
   obj_body = (
      p(_[[The stock tab shows you how many wares you have. Compare the information from the four tabs to understand the correlation.]]) ..
      li(_[[When you have finished, close the ware statistics window.]])
   ),
}

reopen_ware_stats1 = {
   title = _"You closed the ware statistics window!",
   body = (
      p(_[[You have closed the ware statistics menu window, but I have not yet finished with my explanation. Would you please reopen it and choose the third tab?]])
   ),
   show_instantly = true,
   w = 300,
   h = 250
}

reopen_ware_stats1_obj = {
   obj_name = "open_ware_stats_menu_again1",
   obj_title = _"Open the ware statistics window again",
   obj_body = (
      p(_[[You closed the ware statistics menu window before I finished telling you everything about it. If you already know everything, please feel free to leave this tutorial at any time.]]) ..
      -- TRANSLATORS: "it" refers to the ware statistics window.
      li(_[[Otherwise, please reopen it and choose the third tab.]])
   )
}

reopen_ware_stats2 = {
   title = _"You closed the ware statistics window!",
   body = (
      p(_[[You have closed the ware statistics menu window, but I have not yet finished with my explanation. Would you please reopen it and choose the fourth tab?]])
   ),
   show_instantly = true,
   w = 300,
   h = 250
}

reopen_ware_stats2_obj = {
   obj_name = "open_ware_stats_menu_again2",
   obj_title = _"Open the ware statistics window again",
   obj_body = (
      p(_[[You closed the ware statistics menu window before I finished telling you everything about it. If you already know everything, please feel free to leave this tutorial at any time.]]) ..
      -- TRANSLATORS: "it" refers to the ware statistics window.
      li(_[[Otherwise, please reopen it and choose the fourth tab.]])
   )
}

economy_settings1 = {
   position = "topright",
   title = _"Economy options",
   body = (
      p(_[[I’ve shown you our stock menu window, where you could see which wares are at the warehouses. You remember?]]) ..
      p(_[[Now I’ll tell you how you can determine how many wares you want to have. The menu window for this purpose can be accessed via any flag and is called ‘Configure economy’.]]) ..
      -- Yup, that's indeed the correct icon
      li_image("images/wui/stats/genstats_nrwares.png", _[[This is the icon.]]) ..
      li(_[[Open this window.]])
   ),
   h = 350,
   obj_name = "open_economy_settings",
   obj_title = _"Open the ‘Configure economy’ window",
   obj_body = (
      li(_[[Open the ‘Configure economy’ window.]]) ..
      li_arrow(_[[The window can be accessed by clicking on any flag you own.]])
   )
}

economy_settings2 = {
   title = _"Economy options",
   body = (
      p(_[[This window looks similar to the stock window, but it has additional buttons at the bottom.]]) ..
      p(_[[You first have to select one or more wares (you can also left-click and drag). Then you can set the desired target quantity for the selected wares.]]) ..
      p(_[[Most buildings will only produce something when the stock level in your warehouses falls below the target quantity, so you should indicate the reserve you want to stockpile.]]) ..
      p(_[[An example: the default value for scythes is 1. If you build a farm, a carrier will take a scythe and become a farmer. Then there will be no scythes left, but the target quantity is 1, therefore your toolsmith will start producing another one.]]) ..
      p(_[[If you build two farms, only one of them will start working immediately. The second farm will have to wait for its worker, who will lack a scythe. If you had set the target quantity to 2 before, two scythes would have been available and both farms would have been able to start working right away.]])
   ),
   h = 450
}

economy_settings3 = {
   title = _"Economy options",
   body = (
      p(_[[By changing the target quantity, you can therefore decide which wares/tools your resources (in this case: iron) should be turned into or whether you would like to save your iron and wait until you know what you will need it for.]]) ..
      p(_[[Only buildings that consume wares care about this setting. Buildings that produce wares for free (e.g. your farms or wells) will always keep working.]]) ..
      p(_[[Now let’s try it out: the current target quantity for marble columns is 10. Increase it to be prepared in case you will have to build up your fortifications quickly.]]) ..
      li(_[[Set the target quantity for marble columns to 20 and wait for your stonemason to produce them.]])
   ),
   obj_name = "produce_marble_columns",
   obj_title = _"Produce 20 marble columns",
   obj_body = (
      p(_[[Sometimes, you will need many wares at the same time quickly – faster than they can be produced. In this case, it is good to have enough on reserve.]]) ..
      li(_[[To be prepared for additional fortifications, you should produce 20 marble columns.]]) ..
      li_arrow(_[[Your stonemason will not produce marble columns when they are not needed. You have to increase the target quantity.]]) ..
      li_arrow(_[[To do so, click on any flag and choose ‘Configure economy’. In this menu window, you can decide how many wares of each type you wish to have in stock.]])
   )
}

warehouse_preference_settings = {
   field = warehouse_field,
   position = "topright",
   title = _"Warehouse Preferences",
   body = (
      h1(_[[Bring the marble columns to the front line]]) ..
      p(_[[The production of marble columns is working fine now, but it would be great if they were stored where we need them.]]) ..
      p(_[[Normally, produced wares are brought to the closest warehouse if they are not needed elsewhere. In this case, this means our headquarters. But we would like to have them in the warehouse near our fortresses.]]) ..
      p(_[[Every warehouse has four buttons to set the preference. If you move your mouse pointer over them, you will see tooltips that explain what the buttons do.]]) ..
      li(_[[Bring all of the 20 marble columns to the warehouse near the front line.]]) ..
      li_arrow(_[[To achieve this, you will have to do two things. First, set a preference for marble columns in the desired warehouse. All marble columns produced in the future will be brought there if possible.]]) ..
      li_arrow(_[[Then, to move the marble columns out of your headquarters, you will have to click on the remove button there.]])
   ),
   obj_name = "bring_marble_columns_to_front",
   obj_title = _"Bring 20 marble columns to the front line",
   obj_body = (
      p(_[[To decide where your wares get stored, you can use the preference buttons in the warehouses.]]) ..
      li(_[[Bring all of the 20 marble columns to the warehouse near the front line.]]) ..
      li_arrow(_[[To achieve this, you will have to do two things. First, set a preference for marble columns in the desired warehouse. All marble columns produced in the future will be brought there if possible.]]) ..
      li_arrow(_[[Then, to move the marble columns out of your headquarters, you will have to click on the remove button there.]])
   )
}

conclusion = {
   title = _"Borders Secured",
   body =
      h1(_[[We’re safe now]]) ..
      p(_[[Great. We now have enough marble columns so that in case of an aggressor, we can build up our fortifications. But I do not think that that will be necessary. So far, no enemy has shown up.]]) ..
      p(_[[I hope I could teach you how you can control the economy in Widelands. There are many options and they can be confusing at first. Even if you’ve only understood a few concepts, you mustn’t give up. Try them out in some games, become familiar with them and experience the possibilities. Then, return to this tutorial and learn the rest!]]) ..
      p([[]]) ..
      p(_[[This was the last tutorial I had prepared for you. I’ve now taught you everything I know. There are still secrets hidden in this world even I don’t know about. I will now search for a quiet place to spend my sunset years. If you have still questions, the Widelands community will surely help you. You can find it at:]]) ..
      h1(p("align=center", u("widelands.org")))
}
