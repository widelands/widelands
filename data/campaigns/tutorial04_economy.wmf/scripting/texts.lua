-- =======================================================================
--                      Texts for the tutorial mission
-- =======================================================================

-- =============
-- Texts below
-- =============

intro1 = {
   title = _"Your Economy and its Settings",
   body = (
      h1(_[[Economy]]) ..
      li_image("images/wui/stats/genstats_nrwares.png", _[[Welcome back. In this tutorial, I’ll tell you what you can do to check how well your economy works.]]) ..
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
      li_object("empire_fortress",
         p(_[[Now about the map: you have settled in a nice valley between two mountains, rich in marble, iron ore and coal. All were hoping for a peaceful life.]]) ..
         p(_[[But one day, you discovered a barren wasteland with abandoned buildings in the east. A strange aura came from there, and no one wanted to set foot there. But the border could not be left undefended, and so you constructed three fortresses.]]) ..
         p(_[[You had not been prepared for war, and you have to hurry now to build up an army.]]), plr.color)
   ),
   h = 300
}

tavern_burnt_down = {
   position = "topright",
   title = _"An accident",
   body = (
      h1(_[[The Tavern is Burning!]]) ..
      li_object("destroyed_building",
         _[[Oh no, look at this: our tavern is burning! In all the hurry, our innkeeper accidentally dropped a torch. She is fine, but we could not extinguish the fire in time.]], plr.color)
   ),
   w = 300,
   h = 250
}

obj_open_building_stats = {
   name = "open_building_stats",
   title=_"Open the building statistics window",
   number = 1,
   body = objective_text(_"Open the building statistics window",
      li(_[[Open the building statistics window for an overview over the buildings you have.]]) ..
      li_image("images/wui/menus/statistics.png", _[[First, you will have to open the ‘Statistics’ menu at the bottom of the screen.]]) ..
      li_image("images/wui/menus/statistics_buildings.png", _[[Afterwards, choose ‘Buildings’.]]) ..
      li_arrow(_([[You can also use the hotkey ‘%s’.]]):bformat(wl.ui.get_shortcut("game_stats_buildings")))
   )
}
building_stats = {
   position = "topright",
   title = _"Building statistics",
   body = (
      h1(_[[Check out your taverns]]) ..
      p(_[[At first, we should find out how many taverns we currently have. Widelands offers you a window where you can easily check this.]])
   )
}

obj_check_taverns = {
   name = "check_taverns",
   title=_"Look up how many taverns you have",
   number = 1,
   body = objective_text(_"Look up how many taverns you have",
      p(_[[We want to know whether we still have taverns.]]) ..
      li_image("images/wui/fieldaction/menu_tab_buildmedium.png",
         _[[Choose the ‘Medium buildings’ tab in the building statistics window.]]) ..
      li(_[[Look up how many taverns you have.]]) ..
      li_arrow(_[[Below every building, there are two lines. The first one shows the number of buildings you own and how many are under construction. The second line shows the average productivity if it is a production site or training site, or the stationed and desired soldiers in military buildings.]]) ..
      li(_[[Close the building statistics window when you are done.]])
   )
}
explain_building_stats = {
   position = "topright",
   title = _"Building Statistics",
   body = (
      li_image("images/wui/menus/statistics_buildings.png",
         _[[This is the building statistics window. It shows you all buildings you can own, sorted by their size.]]) ..
      p(_[[Let me now explain what all those numbers mean:]]) ..
      li(_[[‘2/1’ below the quarry: This means that you have two quarries, plus another one which is under construction.]]) ..
      li(_[[‘0%’: This indicates the average productivity of all buildings of that type. You have just started this game, therefore none of your buildings has done any work yet, but they are going to start working soon.]]) ..
      li(_[[‘2/4’ below your sentry: For military buildings, the stationed soldiers are shown instead of a productivity. You want to have four soldiers in your sentries, but only two soldiers are stationed in this kind of building. This leaves two vacant positions – we really need more soldiers.]]) ..
      li_arrow(_[[In both cases, the color (green - yellow - red) signals you how good the value is.]]) ..
      li(_[[If you click on a building, you can scroll through the buildings of the selected type.]]) ..
      li(_[[If you don’t have any building of a particular building type, it will be shown greyed out.]]) ..
      p(_[[This is enough explanation for now. Now try it out yourself.]])
   ),
   h = 500,
}

reopen_building_stats = {
   title = _"You closed the building statistics window!",
   body = (
      li_image("images/wui/menus/statistics_buildings.png",
         _[[You have closed the building statistics window. I didn’t notice that you switched to the medium buildings to look up the number of taverns. Would you please be so nice and show it to me?]])
   ),
   show_instantly = true,
   w = 300,
   h = 250
}

obj_reopen_building_stats = {
   name = "reopen_building_stats",
   title = _"Open the building statistics window again",
   number = 1,
   body = objective_text(_"Open the building statistics window again",
      li_image("images/wui/menus/statistics_buildings.png",
         _[[You closed the building statistics window, although you have not yet looked up the number of taverns.]]) ..
      -- TRANSLATORS: "it" refers to the building statistics window.
      li(_[[Please reopen it and choose the second tab (medium buildings).]])
   )
}

obj_open_inventory = {
   name = "open_inventory",
   title=_"Open your stock window",
   number = 1,
   body = objective_text(_"Open your stock window",
      p(_[[The stock window gives you an overview over the wares you currently have.]]) ..
      li_image("images/wui/menus/statistics.png", _[[First, you will have to open the ‘Statistics’ menu at the bottom of the screen.]]) ..
      li_image("images/wui/menus/statistics_stock.png", _[[Afterwards, choose ‘Stock’.]]) ..
      li_arrow(_([[You can also use the hotkey ‘%s’ to access this window quickly.]]):bformat(wl.ui.get_shortcut("game_stats_stock")))
   )
}
inventory1 = {
   position = "topright",
   title = _"Stock",
   body = (
      h1(_[[Check for rations]]) ..
      li_image(wl.Game():get_ware_description("ration").icon_name,
         _[[OK. In the list, you’ve seen that you have no more taverns or inns. That means that you’re not producing any rations. But let’s see what we still have in stock.]])
   )
}

obj_switch_stock_tab = {
   name = "switch_stock_tab",
   title=_"Examine the first two tabs in the stock window",
   number = 1,
   body = objective_text(_"Examine the first two tabs in the stock window",
      p(_[[Have a look at the first two tabs in the stock window. They show all the wares and workers you have.]]) ..
      li_image("images/wui/stats/menu_tab_wares_warehouse.png",
         _[[When you have seen enough, switch back to the third tab (‘Wares in warehouses’).]])
   )
}
inventory2 = {
   position = "topright",
   title = _"Stock",
   body = (
      li_image("images/wui/buildings/menu_tab_wares.png",
         _[[The stock window has four tabs. The first one shows you all your current wares, including those on roads, at flags and inside buildings waiting for processing.]]) ..
      p(_[[Looking at the rations, there are currently only five in total, probably on their way to somewhere. Five rations are not much for such a big economy.]]) ..
      li_image("images/wui/buildings/menu_tab_workers.png",
         _[[The second tab shows you all your workers, again those on roads and in buildings summed up.]])
   ),
   show_instantly = true
}

inventory3 = {
position = "topright",
   title = _"Stock",
   body = (
      li_image("images/wui/stats/menu_tab_wares_warehouse.png",
         _[[The third tab shows you the wares that are stored in your headquarters, your warehouses and ports. They are not needed anywhere and are therefore your reserve.]]) ..
      li_image("images/wui/stats/menu_tab_workers_warehouse.png", _[[The fourth tab shows the same thing for workers.]]) ..
      p(_[[The third tab tells you that there are no rations left in your headquarters – that’s not good!]])
   ),
   show_instantly = true,
   h = 250
}

reopen_stock_menu = {
   title = _"You closed the stock window!",
   body = (
      li_image("images/wui/menus/statistics_stock.png",
         _[[You have closed the stock window, but I have not yet finished with my explanation. Would you please reopen it and choose the first tab?]])
   ),
   show_instantly = true,
   w = 300,
   h = 250
}

obj_reopen_stock_menu = {
   name = "open_stock_menu_again",
   title = _"Open the stock window again",
   number = 1,
   body = objective_text(_"Open the stock window again",
      li_image("images/wui/menus/statistics_stock.png",
         _[[You closed the stock window before I finished telling you everything about it. If you already know everything, please feel free to leave this tutorial at any time.]]) ..
      -- TRANSLATORS: "it" refers to the "Stock" window.
      li(_[[Otherwise, please reopen it and have a look at all its tabs.]])
   )
}

reopen_stock_menu2 = {
   title = _"You closed the stock window!",
   body = (
      li_image("images/wui/menus/statistics_stock.png",
         _[[You have closed the stock window, but I have not yet finished with my explanation. Would you please reopen it and choose the third tab?]])
   ),
   show_instantly = true,
   w = 300,
   h = 250
}
obj_reopen_stock_menu2 = {
   name = "open_stock_menu_again2",
   title = _"Open the stock window again",
   number = 1,
   body = objective_text(_"Open the stock window again",
      li_image("images/wui/menus/statistics_stock.png",
         _[[You closed the stock window before I finished telling you everything about it. If you already know everything, please feel free to leave this tutorial at any time.]]) ..
      -- TRANSLATORS: "it" refers to the "Stock" window.
      li(_[[Otherwise, please reopen it and choose the third tab.]])
   )
}

obj_build_taverns = {
   name = "build_taverns",
   title=_"Build new taverns",
   number = 1,
   body = objective_text(_"Build new taverns",
      li(_[[Build at least two taverns.]]) ..
      li_arrow(_[[As long as we don’t produce rations, our miners won’t dig for ore. And without iron, we cannot forge a single helm.]])
   )
}
build_taverns = {
   position = "topright",
   title = _"New taverns",
   body = (
      h1(_[[We need new taverns]]) ..
      li_object("empire_tavern",
         _[[Now that you have an overview, you should act. I think we should build more than one tavern – two or three are better.]],
         plr.color)
   )
}

obj_open_encyclopedia = {
   name = "open_encyclopedia",
   title=_"Open the in-game help window",
   number = 1,
   body = objective_text(_"Open the in-game help window",
      li_image("images/ui_basic/menu_help.png", _[[The encyclopedia window contains the in-game help and an encyclopedia of the tribe you’re currently playing.]]) ..
      -- TRANSLATORS: "it" refers to the encyclopedia window
      li(_[[Open it. You can access it via the button at the bottom of the screen.]]) ..
      li_arrow(_([[Alternatively, you can access it directly with the ‘%s’ key.]]):bformat(wl.ui.get_shortcut("encyclopedia")))
   )
}
ware_encyclopedia = {
   title = _"Encyclopedia",
   body = (
      h1(_[[How to get help]]) ..
      li_object("empire_soldier",
         _[[Of course, it is difficult to remember all of my remarks and advice. For example, you might ask yourself: ‘Why do we need rations to get soldiers?’]], plr.color) ..
      p(_[[When you’ve played a lot, you will know all these things by heart. But until then or if you’re unsure about your tribe’s needs and abilities and how its buildings and workers operate, you can look it up easily in our tribe-specific in-game help and encyclopedia.]])
   ),
   h = 450,
   show_instantly = true,
}

obj_lookup_wares = {
   name = "lookup_wares",
   title=_"Look up which wares are needed to recruit soldiers",
   number = 1,
   body = objective_text(_"Look up which wares are needed to recruit soldiers",
      p(_[[A soldier needs a wooden spear and a helmet – from there on out, you can search backwards to find the wares and the buildings you need to supply your barracks where the soldier is recruited. When you are finished, just close the encyclopedia window.]]) ..
      li(_[[Use the encyclopedia to find out how to create new soldiers.]]) ..
      li_arrow(_[[Choose the ‘Wares’ tab in the encyclopedia window.]]) ..
      li_arrow(_[[Look up what is needed to produce a helmet and what is needed to produce a wooden spear.]]) ..
      li_arrow(_[[If you want, you may further look up what is needed to produce the wares you just looked up.]]) ..
      li(_[[Close the encyclopedia window when you are done.]])
   )
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
      div("width=100%", div("float=left padding_r=18 padding_t=5 padding_l=4",p(img("images/wui/stats/genstats_nrbuildings.png"))) .. p(_[[The ‘Buildings’ tab contains all the necessary information about the buildings of your tribe.]])) ..
      div("width=100%", div("float=left padding_r=18 padding_t=5 padding_l=4",p(img("tribes/immovables/wheatfield/ripe/menu.png"))) .. p(_[[Finally, the ‘Immovables’ tab shows information about the specific immovables that your tribe’s workers can place on the map.]]))
   ),
   h = 500,
   show_instantly = true,
}

reopen_encyclopedia = {
   title = _"You closed the encyclopedia!",
   body = (
      li_image("images/ui_basic/menu_help.png",
         _[[You have closed the encyclopedia window, but I didn’t notice that you were trying to find out which wares are needed to recruit a soldier. Would you please reopen it and do so?]])
   ),
   show_instantly = true,
   w = 300,
   h = 250
}

obj_reopen_encyclopedia = {
   name = "reopen_encyclopedia",
   title = _"Open the encyclopedia window again",
   number = 1,
   body = objective_text(_"Open the encyclopedia window again",
      li_image("images/ui_basic/menu_help.png",
         _[[You closed the encyclopedia window without searching for the information we need. If you already know everything, please feel free to leave this tutorial at any time.]]) ..
      li(_[[Otherwise, please reopen the encyclopedia window and choose the second tab.]])
   )
}
building_priority_settings = {
   position = "topright",
   title = _"Priority Settings",
   body = (
      h1(_[[Send the wares where they’re needed]]) ..
      li_object("empire_marblemine", p(_[[Great. Our taverns have now been built up and are supplying us with rations.]]) ..
         p(_[[At the moment, all mines are supplied with rations. If you want to prioritize a special mine, you simply have to open its window. In the wares tab, behind every ware, you can see a slider.]]) ..
         p(_[[When you move the slider to the blue area on the left side (low priority), the corresponding ware gets delivered less frequently. Red (right side) means that as many wares as possible should be delivered to this building, maybe because it produces something important.]]) ..
         p(_[[The actual supply state is indicated by varying colors of the ware icons themselves:]]) ..
         li(_[[Wares stored in the building are shown in full color.]]) ..
         li(_[[Missing wares are greyed out.]]) ..
         li(_[[Wares not present in the building yet but already being transported there are shown in a darker grey.]]) ..
         p(_[[In our situation, you might want to work the bakeries as fast as possible because they supply our taverns, so you could set water to the highest priority for them. The other buildings (for example the donkey farm) would then get less water, but the bakery could work faster.]]), plr.color)
      -- we cannot check whether the user does this, so no objective
      -- see https://github.com/widelands/widelands/issues/2012
   )
}

obj_open_ware_stats = {
   name = "open_ware_stats",
   title = _"Open the ware statistics window",
   number = 1,
   body = objective_text(_"Open the ware statistics window",
      li(_[[Select the ‘Wares’ entry from the ‘Statistics’ menu.]])
   )
}
ware_stats1 = {
   title = _"Ware Statistics",
   body = (
      li_image("images/wui/menus/statistics_wares.png", _[[Let’s have a look at how our wares production is doing.]])
   ),
   h = 250
}

obj_switch_ware_stats_tab_to_third = {
   name = "switch_ware_stats_tab_to_third",
   title = _"Examine your ware production and consumption",
   number = 1,
   body = objective_text(_"Examine your ware production and consumption",
      p(_[[The first two tabs show you the production and consumption of any ware. You can toggle them by simply clicking on them.]]) ..
      li(_[[When you have seen enough, switch to the third tab (‘Economy health’).]])
   )
}
ware_stats2 = {
   position = "topright",
   title = _"Ware Statistics",
   body = (
      p(_[[In this window, you can select wares to see how their production or consumption has changed over time. Try it out with some wares.]]) ..
      li_image("images/wui/stats/menu_tab_wares_econ_health.png", _[[I’ll continue as soon as you click on the third tab (‘Economy health’).]])
   ),
   h = 350,
   show_instantly = true
}

obj_switch_ware_stats_tab_to_fourth = {
   name = "switch_ware_stats_tab_to_fourth",
   title = _"Examine your economy’s health",
   number = 1,
   body = objective_text(_"Examine your economy’s health",
      p(_[[The third tab shows you the economy health of the ware. When the value is positive, this means your stock is growing.]]) ..
      p(_[[Now try this out. You can also compare it with the two previous tabs.]]) ..
      li(_[[When you have seen enough, switch to the fourth tab (‘Stock’).]])
   )
}
ware_stats3 = {
   position = "topright",
   title = _"Ware Statistics",
   body = (
      li_image("images/wui/stats/menu_tab_wares_econ_health.png",
         _[[In this tab, you can see the difference between production and consumption, called ‘economy health’. You can see at one glance which one of those two is higher for the selected ware, that means whether the amount increases or decreases.]])
   ),
   h = 350,
   show_instantly = true
}

obj_close_ware_stats = {
   name = "close_ware_stats",
   title = _"Examine your stock",
   number = 1,
   body = objective_text(_"Examine your stock",
      li_arrow(_[[The stock tab shows you how many wares you have. Compare the information from the four tabs to understand the correlation.]]) ..
      li(_[[When you have finished, close the ware statistics window.]])
   ),
   h = 250
}
ware_stats4 = {
   position = "topright",
   title = _"Ware Statistics",
   body = (
      li_image("images/wui/stats/menu_tab_wares_stock.png",
         p(_[[In the last tab, you can also see your absolute stock. It will increase when the economy health is positive, and decrease otherwise. Compare the two graphs!]]) ..
         p(_[[The last two tabs are good indicators to see whether shortages are about to come. Don’t forget to check them regularly!]]))
   ),
   show_instantly = true
}

reopen_ware_stats1 = {
   title = _"You closed the ware statistics window!",
   body = (
      li_image("images/wui/menus/statistics_wares.png",
         _[[You have closed the ware statistics window, but I have not yet finished with my explanation. Would you please reopen it and choose the third tab?]])
   ),
   show_instantly = true,
   w = 300,
   h = 250
}

obj_reopen_ware_stats1 = {
   obj_name = "reopen_ware_stats1",
   obj_title = _"Open the ware statistics window again",
   number = 1,
   obj_body = (
      li_image("images/wui/menus/statistics_wares.png",
         _[[You closed the ware statistics window before I finished telling you everything about it. If you already know everything, please feel free to leave this tutorial at any time.]]) ..
      -- TRANSLATORS: "it" refers to the ware statistics window.
      li(_[[Otherwise, please reopen it and choose the third tab.]])
   )
}

reopen_ware_stats2 = {
   title = _"You closed the ware statistics window!",
   body = (
      li_image("images/wui/menus/statistics_wares.png",
         _[[You have closed the ware statistics window, but I have not yet finished with my explanation. Would you please reopen it and choose the fourth tab?]])
   ),
   show_instantly = true,
   w = 300,
   h = 250
}

obj_reopen_ware_stats2 = {
   name = "open_ware_stats_menu_again2",
   title = _"Open the ware statistics window again",
   number = 1,
   body = (
      li_image("images/wui/menus/statistics_wares.png",
         _[[You closed the ware statistics window before I finished telling you everything about it. If you already know everything, please feel free to leave this tutorial at any time.]]) ..
      -- TRANSLATORS: "it" refers to the ware statistics window.
      li(_[[Otherwise, please reopen it and choose the fourth tab.]])
   )
}

obj_open_economy_settings = {
   name = "open_economy_settings",
   title = _"Open the ‘Configure economy’ window",
   number = 1,
   body = objective_text(_"Open the ‘Configure economy’ window",
      li(_[[Open the ‘Configure economy’ window.]]) ..
      li_arrow(_[[The window can be accessed by clicking on any flag you own.]])
   )
}
economy_settings1 = {
   position = "topright",
   title = _"Economy options",
   body = (
      li_object("empire_flag",
         p(_[[I’ve shown you our stock window, where you could see which wares are at the warehouses. You remember?]]) ..
         p(_[[Now I’ll tell you how you can determine how many wares you want to have. The window for this purpose can be accessed via any flag and is called ‘Configure economy’.]]), plr.color) ..
      -- Yup, that's indeed the correct icon
      li_image("images/wui/stats/genstats_nrwares.png", _[[This is the icon.]])
   ),
   h = 350
}

economy_settings2 = {
   position = "topright",
   title = _"Economy options",
   body = (
      li_image("images/wui/stats/genstats_nrwares.png",
            _[[This window looks similar to the stock window, but it has additional buttons at the bottom.]]) ..
      p(_[[You first have to select one or more wares (you can also left-click and drag). Then you can set the desired target quantity for the selected wares.]]) ..
      p(_[[Most buildings will only produce something when the stock level in your warehouses falls below the target quantity, so you should indicate the reserve you want to stockpile.]]) ..
      li_image(wl.Game():get_ware_description("scythe").icon_name,
         _[[An example: the default value for scythes is 1. If you build a farm, a carrier will take a scythe and become a farmer. Then there will be no scythes left, but the target quantity is 1, therefore your toolsmith will start producing another one.]]) ..
      li_object("empire_farmer",
         _[[If you build two farms, only one of them will start working immediately. The second farm will have to wait for its worker, who will lack a scythe. If you had set the target quantity to 2 before, two scythes would have been available and both farms would have been able to start working right away.]], plr.color)
   )
}

obj_produce_marble_columns = {
   name = "produce_marble_columns",
   title = _"Produce 20 marble columns",
   number = 1,
   body = objective_text(_"Produce 20 marble columns",
      li(_[[To be prepared for additional fortifications, you should produce 20 marble columns.]]) ..
      li_arrow(_[[Your stonemason will not produce marble columns when they are not needed. You have to increase the target quantity.]]) ..
      li_arrow(_[[To do so, click on any flag and choose ‘Configure economy’. In this window, you can decide how many wares of each type you wish to have in stock.]]) ..
      li_arrow(_[[Sometimes, you will need many wares at the same time quickly – faster than they can be produced. In this case, it is good to have enough on reserve.]])
   )
}
economy_settings3 = {
   position = "topright",
   title = _"Economy options",
   body = (
      li_image(wl.Game():get_ware_description("iron").icon_name,
         _[[By changing the target quantity, you can therefore decide which wares/tools your resources (in this case: iron) should be turned into or whether you would like to save your iron and wait until you know what you will need it for.]]) ..
      p(_[[Only buildings that consume wares care about this setting. Buildings that produce wares for free (e.g. your farms or wells) will always keep working.]]) ..
      li_image(wl.Game():get_ware_description("marble_column").icon_name,
         _[[Now let’s try it out: the current target quantity for marble columns is 10. Increase it to be prepared in case you will have to build up your fortifications quickly.]])
   ),
   h = 500,
}

obj_bring_marble_columns_to_front = {
   name = "bring_marble_columns_to_front",
   title = _"Bring 20 marble columns to the front line",
   number = 1,
   body = objective_text(_"Bring 20 marble columns to the front line",
      li(_[[Bring all of the 20 marble columns to the warehouse near the front line.]]) ..
      li_image("images/wui/buildings/stock_policy_button_prefer.png",
         _[[To achieve this, you will have to do two things. First, set a preference for marble columns in the desired warehouse. All marble columns produced in the future will be brought there if possible.]]) ..
      li_image("images/wui/buildings/stock_policy_button_remove.png",
         _[[Then, to move the marble columns out of your headquarters, you will have to click on the remove button there.]])
   )
}
warehouse_preference_settings = {
   field = warehouse_field,
   position = "topright",
   title = _"Warehouse Preferences",
   body = (
      h1(_[[Bring the marble columns to the front line]]) ..
      li_object("empire_warehouse",
         p(_[[The production of marble columns is working fine now, but it would be great if they were stored where we need them.]]) ..
         p(_[[Normally, produced wares are brought to the closest warehouse if they are not needed elsewhere. In this case, this means our headquarters. But we would like to have them in the warehouse near our fortresses.]]) ..
         p(_[[Every warehouse has four buttons to set the preference. If you move your mouse pointer over them, you will see tooltips that explain what the buttons do.]]), plr.color)
   ),
   h = 500
}

conclusion = {
   title = _"Borders Secured",
   body =
      h1(_[[We’re safe now]]) ..
      li_object("empire_fortress",
         p(_[[Great. We now have enough marble columns so that in case of an aggressor, we can build up our fortifications. But I do not think that that will be necessary. So far, no enemy has shown up.]]) ..
         p(_[[I hope I could teach you how you can control the economy in Widelands. There are many options and they can be confusing at first. Even if you’ve only understood a few concepts, you mustn’t give up. Try them out in some games, become familiar with them and experience the possibilities. Then, return to this tutorial and learn the rest!]]), plr.color) ..
      p([[]]) ..
      p(_[[This was the last tutorial I had prepared for you. I’ve now taught you everything I know. There are still secrets hidden in this world even I don’t know about. I will now search for a quiet place to spend my sunset years. If you have still questions, the Widelands community will surely help you. You can find it at:]]) ..
      h1(p("align=center", u("widelands.org"))),
   allow_next_scenario = true,
   h = 450
}
