-- =========================
-- Some formating functions
-- =========================

function lutius(title, text)
   return speech("map:Lutius.png", styles.color("campaign_emp_lutius"), title, text)
end
function saledus2(title, text)
   return speech("map:Saledus2.png", styles.color("campaign_emp_saledus"), title, text)
end
function saledus3(title, text)
   return speech("map:Saledus3.png", styles.color("campaign_emp_saledus"), title, text)
end
function saledus4(title, text)
   return speech("map:Saledus2.png", styles.color("campaign_emp_saledus"), title, text)
end
function amalea1(title, text)
   return speech("map:Amalea.png", styles.color("campaign_emp_amalea"), title, text)
end
function amalea2(title, text)
   return speech("map:Amalea2.png", styles.color("campaign_emp_amalea"), title, text)
end
function amalea3(title, text)
   return speech("map:Amalea3.png", styles.color("campaign_emp_amalea"), title, text)
end

function holy_shrine(text, image)
   return
      div("width=100%",
         div("width=60%", p(text)) ..
         div("width=*", p("align=left", img(image)))
      )
end

-- =======================================================================
--                           Texts - No logic here
-- =======================================================================


-- ===========
-- Objectives
-- ===========
obj_build_first_outpost = {
    name = "build_a_first_outpost",
    title=_("Build your first outpost"),
    number = 1,
    body = objective_text(_("First Outpost"),
       li(_([[Build an outpost at the yellow house symbol to the east of your provisional headquarters.]]))
    ),
}

obj_build_quarry = {
   name = "build_quarry",
   title=_("Build a quarry"),
   number = 1,
   body = objective_text(_("Quarry"),
      li(_([[Build a quarry close to the abundant rocks to free some building plots and get some valuable resources.]]))
   ),
}

obj_build_sawmill_stonemason_and_lumberjacks = {
   name = "build_sawmill_stonemason_and_lumberjacks",
   title=_("Build a lumberjack’s house, a sawmill and a stonemason’s house"),
   number = 3,
   body = objective_text(_("Lumberjack, Sawmill and Stonemason"),
      li(_([[Build a lumberjack’s house, a sawmill and a stonemason’s house as soon as there is enough space for them. This will complete the basic building material supply chain.]]))
   ),
}

obj_lower_marble_column_demand = {
   name = "lower_marble_column_demand",
   title=_("Lower the demand for marble columns to 4"),
   number = 1,
   body = objective_text(_("Lower Demand for Marble Columns"),
      li(_([[Lower the demand for marble columns in the economy settings to four. This will help to save some marble for buildings.]])) ..
      li_arrow(_([[Remember that you can access the economy settings by clicking on any flag.]]))
   ),
}

obj_find_port_space = {
   name = "find_port_space",
   title=_("Find a port space on the small island"),
   number = 1,
   body = objective_text(_("Port Space"),
      li(_([[Search for a port space in order to regain access to the sea.]]))
   ),
}

obj_build_port_and_shipyard = {
   name = "build_port_and_shipyard",
   title=_("Build a port and a shipyard on the Island"),
   number = 2,
   body = objective_text(_("Ship Industry"),
      li(_([[Build a port to set sail from this island.]]))..
      li(_([[Build a shipyard to produce the ships to sail on.]]))),
}

obj_produce_wheat = {
   name = "produce_50_wheat",
   title=_("Produce wheat"),
   number = 1,
   body = objective_text(_("Wheat Production"),
      li(_([[Produce at least 50 sheaves of wheat to get your wool production started.]]))
   ),
}

obj_produce_cloth = {
   name = "produce_cloth",
   title=_("Produce cloth for building our first ship"),
   number = 1,
   body = objective_text(_("Cloth Production"),
      li(_([[Now build a sheep farm and turn all the wheat into at least 50 bales of wool. Afterwards use the wool to weave cloth in order to build a ship.]]))
   ),
}

obj_build_expedition_ports = {
   name = "build_expedition_ports",
   title=_("Build 2 ports on distant islands"),
   number = 1,
   -- TRANSLATORS: Objective - create 2 expeditions and found new colonies with them
   body = objective_text(_("Build two Expedition Ports"),
      li(_([[Explore the island in the south. Send an expedition from our port to build a port there. Afterwards, try to find some mining resources.]])) ..
      li(_([[Send a ship northwards to gather some intelligence about the Barbarians. Try to build a port on the island and found an outpost.]]))
   ),
}

obj_produce_gold = {
   name = "produce_gold",
   title=_("Produce gold to send out expeditions"),
   number = 1,
   body = objective_text(_("Gold Production"),
      li(_([[Produce at least six bars of gold to be able to colonize your vicinity.]]))
   ),
}

obj_conquer_all = {
   name = "conquer_all",
   title=_("Defeat the Barbarians"),
   number = 1,
   body = objective_text(_("Defeat the Enemy"),
      li(_([[End the Barbarians’ existence in this part of the world.]]))
   ),
}

obj_find_all_ports = {
   name = "explore_futher_build_ports",
   title=_("Explore further and build ports"),
   number = 1,
   body = objective_text(_("Explore Additional Islands and Search for Port Spaces"),
      li(_([[Discover more land in your vicinity and build as many ports as you can.]]))
   ),
}

obj_find_artifacts = {
   name = "find_neptunes_shrine",
   -- TRANSLATORS: Objective header
   title=_("Recover the shrine"),
   number = 1,
   body = holy_shrine(
      objective_text(
         _("Find the Holy Shrine"),
         _([[We need to find and secure all six pieces of the holy shrine of Neptune.]])
      ),
      "map:artifacts_models.png"
   )
}

obj_training = {
   name = "build_training_infrastructure",
   title=_("Build a training infrastructure"),
   number = 3,
   body = objective_text(_("Training Infrastructure"),
      li(_([[Build a training camp, some barracks and an arena or colosseum to train your soldiers.]]))
   ),
}


-- ==================
-- Texts to the user
-- ==================
diary_page_1 = {
   title =_("The Journey Home"),
   body=lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Finally, we were set towards Fremil again. I couldn’t await the chance to clear my records of this big defeat by the Barbarians.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[But the gods themselves didn’t want us to get home so quickly!]])),
   posy=1,
   w=550,
}

diary_page_2 = {
   title=_("The Wrath of Neptune"),
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Neptune himself was against us and drove us into a dark, wild storm again. But this time it felt even worse than last time. The waves were twice the height of our brave ship and the winds drove us into unsafe waters.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[Our ship is heavily damaged. The mast has gone overboard and the hull is taking in more and more water. We would be glad to survive this black night with barely more than our lives.]])),
   posy=1,
   w=550,
}

diary_page_3 = {
   title=_("Shipwrecked Again"),
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Finally, the wrath of the gods and the ocean seems to be over.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[Again, we stranded on an unknown coast and slept in the shade under some palm trees until the sun was risen high up in the sky. I’m afraid that when we awoke at noon, nobody was really surprised that all we could see was sand… although the amount of rocks near our camp was astonishing.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[It really seems as if we have landed on a very small island with little space.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[To make matters even worse, all the land we can see is blocked by rocks and trees. I fear we won’t find the resources we need to build a new ship.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[I really don’t know if and how we will get back home. And the war is going on without us. I hope my fellow soldiers are defending our empire well.]])),
   posy=1,
   w=550,
}

saledus = {
   title=_("No Sight"),
   body= saledus2(_("Saledus looks around nervously…"),
      -- TRANSLATORS: Saledus
      _([[Sire, we do not know anything about the island. And I believe we are not safe either. It would probably be reasonable to build a tower to explore the island and add some military strength.]])),
   posy=1,
   w=550,
}

amalea = {
   title=_("Amalea Interrupts"),
   body= amalea1(_("Amalea raises her hand…"),
      -- TRANSLATORS: Amalea
      _([[As we don’t know how many resources we can get, we should be very restrictive in spending them. Even for military buildings, we should be very careful.]])),
   posy=1,
   w=550,
}

saledus_1 = {
   -- TRANSLATORS: Message header - Saledus is advising to save resources
   title=_("Save Resources"),
   body= saledus2(_("Saledus is nodding in agreement…"),
      -- TRANSLATORS: Saledus
      _([[Sire, Amalea is right about the resources. Therefore I correct myself. We should only build an outpost as this is the most effective military building.]]))
      .. new_objectives(obj_build_first_outpost),
   posy=1,
   w=550,
   h=480,
}

saledus_2 = {
   title=_("Rocks, Nothing but Rocks"),
   body= saledus3(_("Saledus speaks with a sigh of relief…"),
      -- TRANSLATORS: Saledus
      _([[Sire, I saw that the construction of the outpost was completed, so I have assigned some of my best soldiers to it to keep the watch.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[But although this is a good step forward, I can’t see much space for buildings. The whole island is littered with rocks. How can we ever build a new ship or even have some basic infrastructure to make our lives more comfortable?]])),
   posy=1,
   w=550,
}

amalea_1 = {
   title=_("Amalea’s Plan"),
   body= amalea2(_("Amalea smiles…"),
      -- TRANSLATORS: Amalea
      _([[Lutius, this may seem very tricky, but we can do it. First of all, we need patience as well as speed and a plan to succeed in this part of the world.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I think we should try to reveal some building space by cutting up all the rocks in a quarry.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Additionally, this would gain us some granite and some marble to build up our new world. We can use them as soon as there will finally be enough space for buildings.]]))
      .. new_objectives(obj_build_quarry),
   posy=1,
   w=550,
   h=480,
}

amalea_2 = {
   title=_("Amalea’s New Instructions"),
   body= amalea3(_("Amalea recommends…"),
      -- TRANSLATORS: Amalea
      _([[Good news from the people, Lutius. I got the message that our first quarry has started its work today.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Perhaps it would be a good idea to wait until it has cleared some space and then build up a basic construction material industry.]]))
      .. new_objectives(obj_build_sawmill_stonemason_and_lumberjacks),
   posy=1,
   w=550,
   h=500,
}

amalea_3 = {
   title=_("Amalea’s Recommendations"),
   body= amalea2(_("Amalea comes in…"),
      -- TRANSLATORS: Amalea
      _([[I’ve got important things to talk about… First the good news:]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I noticed that our stonemason has started his work, finally. He will cut the number of marble columns that we request in our basic economy settings.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But here’s the rub, if we use up all of our marble for columns, we might run out. So, we should lower the settings for marble columns in the economy settings to a smaller amount. I think a reserve of four columns might be sufficient for the moment being.]]))
      .. new_objectives(obj_lower_marble_column_demand),
   posy=1,
   w=550,
   h=500,
}

amalea_10 = {
   title=_("Amalea Looks Happy"),
   body= amalea2(_("Amalea comes in…"),
      -- TRANSLATORS: Amalea
      _([[I have excellent progress to report now.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I found out that the construction of the lumberjack’s house and the sawmill is complete, so we can begin to harvest logs and make planks out of them.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[This is a great step forward in constructing advanced buildings. And we can think of using some planks to build new ships.]])),
   posy=1,
   w=550,
}

saledus_3 = {
   title=_("We Need a Port"),
   body= saledus2(_("Saledus is thoughtful…"),
      -- TRANSLATORS: Saledus
      _([[Sire, I just pondered our way home. It is of utmost importance to build a port for loading our ships and starting an expedition.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[The problem is that a port is a building which needs a very special place to be built. So, while building our economy, we should nevertheless expand further to discover the whole island. Let’s try to find such a valuable construction site.]]))
      .. new_objectives(obj_find_port_space),
   posy=1,
   w=550,
   h=480,
}

saledus_4 = {
   title=_("A Step Closer to Getting Home"),
   body= saledus3(_("Saledus looks excited…"),
      -- TRANSLATORS: Saledus
      _([[Sire, today we got a lot closer to finally returning home again. Our military has spotted a space to build a port on. We should start to build a port quickly.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[Now we truly can look forward to building some ships. So, we should build a shipyard as well.]]))
      .. new_objectives(obj_build_port_and_shipyard),
   posy=1,
   w=550,
   h=500,
}

amalea_4 = {
   title=_("Amalea’s Wisdom"),
   body= amalea2(_("Amalea recommends…"),
      -- TRANSLATORS: Amalea
      _([[I understand the euphoria of Saledus, but we have some serious problems to solve first.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[To send out an expeditionary ship, we first need a ship. Unfortunately, we have only five bales of cloth left, which we will need to build the port and the shipyard.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[So, we need more cloth to finally be able to build ships. But there is no space on the island for building a whole economy to produce cloth. Therefore, we need to build the necessary wares step by step.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[So, let’s think this over. To weave cloth we need some wool from a sheep farm. The sheep farm itself needs wheat and water to work. So, let’s find a space for a big building and build a farm first.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[After the production of enough wheat, we can dismantle the farm and build a sheep farm to produce wool. When the sheep farm starts working, we could find or free up space for a medium building to build a weaving mill to use our wool for weaving cloth. But for the moment being, let’s start by producing a reasonable amount of wheat.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I think a stock of 50 sheaves of wheat would be enough for starters. We do not have a target quantity for wheat, because the farmer does not need other wares to produce it. All we have to do is watch our stock of wheat grow.]]))
      .. new_objectives(obj_produce_wheat),
   posy=1,
   w=550,
   h=550,
}

diary_page_4 = {
   title=_("Seafaring is Possible Again"),
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Praise the gods. We finally have a ship industry.]])),
   posy=1,
   w=550,
}

amalea_5 = {
   title=_("Amalea is Proud"),
   body= amalea2(_("Amalea cheers…"),
      -- TRANSLATORS: Amalea
      _([[Well done so far. We have produced enough wheat. Now dismantle the farm and build the next building in the production chain.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Don’t forget to check the target quantity of wool and cloth in the economy settings. We need to turn all the wheat into wool and then the all the wool into cloth.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Perhaps it would be a good idea to stop the shipyard’s production until we have some cloth available in order to not waste any logs or planks.]]))
      .. new_objectives(obj_produce_cloth),
   posy=1,
   w=550,
   h=480,
}

saledus_5 = {
   title=_("We Should Explore the Islands"),
   body= saledus2(_("Saledus is excited…"),
      -- TRANSLATORS: Saledus
      _([[Now that we have all resources ready for building a ship, we should explore the surrounding islands.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[First, we need to find some more resources to back up our growing economy. I have spotted an island with some mountains in the south. Perhaps we can find some valuable resources like iron there.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[But the following objective is even more important:]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[I think I have heard the wind carrying some weird but well known sounds from the north. If I’m not totally wrong, I have heard some Barbarian war drums. So, we should better go and have a look at what is going on up there.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[In any case we should be careful with our ports. To defend them against possible enemies we should send some soldiers to each new port, until we can protect them with some military buildings.]]))
      .. new_objectives(obj_build_expedition_ports),
   posy=1,
   w=550,
   h=550,
}

amalea_6 = {
   title=_("Amalea Advises"),
   body= amalea3(_("Amalea looks thoughtful…"),
      -- TRANSLATORS: Amalea
      _([[Salve Lutius. While talking to all our brave builders, they told me that they have to use all our remaining gold for building the port. So, we won’t have any gold left after our port is built.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But the problem is that for sending out an expedition, we need some gold.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Fortunately, I spotted a mountain which looks very promising and seems to have some mining resources. Perhaps we should send some geologists to explore the hidden resources.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I strongly recommend building a production chain for mining and melting some gold. I think we need a vineyard, a winery, a tavern, a fisher’s house or a hunter’s house and a smelting works. And the mines themselves of course.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[If there are difficulties with building all the buildings needed, we should think about all we have learned so far. We can cut down all the rocks. We should perhaps optimize our roads to free up some building sites, or even dismantle some military buildings to obtain more space for our economy.]]))
      .. new_objectives(obj_produce_gold),
   posy=1,
   w=550,
   h=550,
}

saledus_6 = {
   title=_("We Should Conquer All the Islands"),
   body= saledus3(_("Saledus is excited…"),
      -- TRANSLATORS: Saledus
      _([[Now it is time to conquer the islands.]])
      .. paragraphdivider() ..
      _([[Let’s finish the Barbarians off and provide a nice colony to the caesar of our empire.]]))
      .. new_objectives(obj_conquer_all),
   posy=1,
   w=550,
   h=480,
}

amalea_7 = {
   title=_("Amalea’s Luck"),
   body= amalea2(_("Amalea is really proud of the achievements…"),
      -- TRANSLATORS: Amalea
      _([[Lutius, we have just solved another problem. We managed to melt enough gold to send out at least three expeditions. Perhaps we might even keep some gold to make some jewelry.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I’m just kidding. Of course we should spare as much gold as we can for building an arena and other buildings to train our soldiers.]])),
   posy=1,
   w=550,
}

amalea_8 = {
   title=_("Amalea is Pleased"),
   body= amalea1(_("Amalea is content that we will manage all our challenges"),
      -- TRANSLATORS: Amalea
      _([[Well done so far. Now we will not produce more marble columns than needed. I really think we will need this economy feature quite often to adjust our economy. Remember, our industry will only be working until the given amount in the economy settings is reached.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[For example, it might be a good idea to produce a reasonable amount of a ware and afterwards dismantle the building to gain space for another building.]])),
   posy=1,
   w=550,
}

amalea_9 = {
   title=_("Amalea’s Reminder"),
   body= amalea3(_("Amalea is shaking her head…"),
      -- TRANSLATORS: Amalea
      _([[Lutius. I really appreciate that you and Saledus are so concerned about our security. But I think we could use the little space on this island better than by building a lot of military buildings.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I strongly recommend dismantling most of them to be able to build industrial buildings. I don’t think there is much danger on this island, but if you want to be cautious, you can keep some sentries or blockhouses near the coastline.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But be careful not to waste any bigger building spots.]])),
   posy=1,
   w=550,
}

amalea_11 = {
   title=_("Amalea’s Excited"),
   body= amalea1(_("Amalea is looking forward to the exploration…"),
      -- TRANSLATORS: Amalea
      _([[Lutius. We have made so much progress so far. I am glad to hear that we have many ships now.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Now that our fleet is growing and growing we should discover the complete region. Let’s try to find more land and eventually get more resources.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[It would be especially helpful to find more marble, either within fields of rock or in the mountains to mine it.]]))
      .. new_objectives(obj_find_all_ports),
   posy=1,
   w=550,
   h=480,
}

amalea_12 = {
   title=_("Amalea is Very Content"),
   body= amalea2(_("Amalea is clapping her hands…"),
      -- TRANSLATORS: Amalea
      _([[Lutius. Well done. Well done. We have discovered all possible port locations in the complete region. Now we can strengthen our economy and we will manage to sail home soon.]])),
   posy=1,
   w=550,
}

amalea_13 = {
   title=_("Amalea’s Counsel"),
   body= amalea1(_("Amalea is clapping her hands…"),
      -- TRANSLATORS: Amalea
      _([[Salve Lutius. We just completed our weaving mill. Now we are very close to taking to the sea again. However, I thought it would be helpful to remind you that ship production is not the only industry that needs cloth.]])
      .. paragraphdivider() ..
      _([[We need some cloth as well for producing armor in our armor smithy. We need it to build some of the training buildings for our soldiers. And most important and most imminent of all, we need it to send out expeditions.]])
      .. paragraphdivider() ..
      _([[Therefore, I really recommend that we check our stock of cloth quite regularly to ensure that we keep enough cloth in reserve to send out some expeditions at least. As one expedition needs three bales of cloth, I think a reserve of six to nine bales giving us two to three expeditions would be sufficient.]])),
   posy=1,
   w=550,
}

saledus_7 = {
   title=_("Victory is Ours"),
   body= saledus3(_("Saledus is cheering…"),
      -- TRANSLATORS: Saledus
      _([[Sire, finally we have defeated the Barbarians in this region. And furthermore, we have built a nice colony for our empire.]])
      .. paragraphdivider() ..
      _([[The king will be very pleased with our victory.]])),
   posy=1,
   w=550,
}

saledus_8 = {
   title=_("Pleasing Neptune"),
   body= saledus4(_("Saledus is Very Sad and Anxious…"),
      -- TRANSLATORS: Saledus
      _([[Sire, we are making good progress towards getting home eventually. But there is one thing I need to talk about:]])
      .. paragraphdivider() ..
      _([[As we have seen on our journeys, it is of great importance to put the gods and especially Neptune in a good mood to safeguard our journey.]])
      .. paragraphdivider() ..
      _([[Unfortunately, we lost our holy shrine of Neptune in the great storm, so we can’t worship him well. I think we should search the whole region to see if we can recover all six pieces of the shrine.]])
      .. paragraphdivider() ..
      _([[To ease the search, I have crafted some models of the pieces as you can see. You can give them to our soldiers and sailors to look for them.]]))
      .. new_objectives(obj_find_artifacts),
   posy=1,
   w=550,
   h=550,
}

saledus_9 = {
   title=_("Praise, Praise, Praise"),
   body= saledus2(_("Saledus is happily worshipping…"),
      -- TRANSLATORS: Saledus
      _([[Sire, we recovered the holy shrine of Neptune. Now we can seek his protection for our journey.]])
      .. paragraphdivider() ..
      _([[Oh great Neptune, safeguard our journey! Oh great Neptune, let the winds be friendly to our course! Oh great Neptune, save us from the creatures of the sea! Oh great Neptune, …]])),
   posy=1,
   w=550,
}

saledus_12 = {
   title=_("Unpleasant Discoveries"),
   body= saledus3(_("Saledus looks very worried…"),
      -- TRANSLATORS: Saledus
      _([[Sire, we found the ruins of a former Barbarian fortress and village. I’m somewhat worried that we will have to fight against them before we are ready to do so.]])),
   posy=1,
   w=550,
}

saledus_10 = {
   title=_("Train Faster, Train Harder"),
   body= saledus2(_("Saledus is demanding some discipline"),
      -- TRANSLATORS: Saledus
      _([[Sire, we have all buildings available for training our soldiers, but we should increase our efforts to train them to the highest level and to train more of them.]])
      .. paragraphdivider() ..
      _([[We need a lot of food, a lot of iron and a lot of gold to do so. So, we have lots of challenges to cope with.]])),
   posy=1,
   w=550,
}

saledus_11 = {
   title=_("Training Infrastructure is Needed"),
   body= saledus3(_("Saledus has security concerns"),
      -- TRANSLATORS: Saledus
      _([[Sire, we need to train some of our people to serve in your great army. It is important to increase the number of our soldiers as fast as we can. And we should train them well to improve their fighting abilities.]])
      .. paragraphdivider() ..
      _([[Therefore, we need to build a full training infrastructure despite the limited space available. We need some barracks, a training camp and an arena or a colosseum.]]))
      .. new_objectives(obj_training),
   posy=1,
   w=550,
   h=480,
}

diary_page_5 = {
   title=_("Time to get Home Again"),
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Yes my fellow soldiers, we have done it! We have met all the challenges that were put in front of us. But my heart is still with our home. We shall set sail as soon as possible to help defend the Empire from the Barbarian tribe.]]))
      .. objective_text(_("Victory"),
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   posy=1,
   w=550,
   allow_next_scenario = true,
}

function diary_page_6(first_message, missing)
   push_textdomain("scenario_emp03.wmf")
   local text = ""
   if first_message then
   -- TRANSLATORS: Lutius - Diary
      text = _([[Finally, we have discovered a piece of our shrine. Praise Neptune!]])
   else
   -- TRANSLATORS: Lutius - Diary
      text = _([[We have discovered another piece of our shrine. Praise Neptune!]])
   end
   if missing > 0 then
      text = text .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      ngettext([[We still need to discover %d piece of our shrine.]], [[We still need to discover %d pieces of our shrine.]], missing):bformat(missing)
   else
   -- TRANSLATORS: Lutius - Diary
      text = _([[Finally, we have discovered the last piece of our shrine. Now we can reassemble it to worship our god. Praise Neptune!]])
   end
   local r = {
      title=_("Artifact Found"),
      body= lutius(_("Diary of Lutius"), text),
      posy=1,
      w=550,
   }
   pop_textdomain()
   return r
end
