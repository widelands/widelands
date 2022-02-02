-- =========================
-- Some formating functions
-- =========================

function lutius(title, text)
   return speech("map:Lutius.png", "2F9131", title, text)
end
function saledus(title, text)
   return speech("map:Saledus.png", "8F9131", title, text)
end
function amalea(title, text)
   return speech("map:Amalea.png", "AF7511", title, text)
end

-- =======================================================================
--                           Texts - No logic here
-- =======================================================================


-- ===========
-- objectives
-- ===========
obj_build_blockhouse = {
   name = "build_blockhouse",
   title=_("Build a blockhouse"),
   number = 1,
   body = objective_text(_("Blockhouse"),
      li(_([[Build a blockhouse at the red house symbol on the east side of the forests, to the right of your provisional headquarters.]]))
   ),
}

obj_build_lumberjack = {
   name = "build_lumberjack",
   title=_("Build a lumberjack’s house"),
   number = 1,
   body = objective_text(_("Lumberjack’s House"),
      li(_([[Build a lumberjack’s house at the red house symbol, south of your provisional headquarters.]]))
   ),
}

obj_build_sawmill_and_lumberjacks = {
   name = "build_sawmill_and_lumberjacks",
   title=_("Build 2 lumberjack’s houses and a sawmill"),
   number = 3,
   body = objective_text(_("Two Lumberjack’s Houses and a Sawmill"),
      li(_([[Build two more lumberjack’s houses and a sawmill as soon as there is enough space for them.]]))
   ),
}

obj_build_forester = {
   name = "build_forester",
   title=_("Build a forester’s house"),
   number = 1,
   body = objective_text(_("Forester’s House"),
      li(_([[Build a forester’s house to preserve the wood resources of this island.]]))
   ),
}

obj_build_quarry = {
   name = "build_quarry",
   title=_("Build a quarry"),
   number = 5,
   body = objective_text(_("Quarry"),
      li(_([[Build a quarry in the south to cut some granite and marble out of the rocks.]])) ..
      li_arrow(_([[These might be used for future buildings.]]))
   ),
}



-- ==================
-- Texts to the user
-- ==================
diary_page_1 = {
   title =_("A Dark Night"),
   body=lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[What has become of our Empire? I really ask this question – why did my king forbid me from fighting against that monstrous Barbarian tribe, who first acted as a peaceful friend and then attacked my army in the darkest night?]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[With an army of 150 men, I was assigned to patrol on our northern frontier, which lies near the Galdin Mountains in a great, beautiful and ancient forest. Soon, we met a Barbarian tribe, which at first was friendly. A few of my men even traded with them and their children came to us, to admire our clean and tidy uniforms and weapons.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[But during our fifth night in that region, they attacked us with no reason. I lost dozens of good men, and found myself imprisoned and brought to their chieftain. He spat at me, mocked me and told me to leave this land forever. He did not want new land and did not want our land, but he told me that THIS forest was their land – and it would stay theirs until the spirits took the last man of his folk!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[Three weeks later, I returned to Fremil to speak with our king. He was furious at the Barbarians, yet forbade any attack on these tribes. He believed that our army was too weak to survive in a war against the Barbarians.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary. Gulf of Perl is a place name.
      _([[Still, I saw no way that I could erase my shame without fighting against these Barbarians. So, I left Fremil by boat to find a new world and a new life for myself, somewhere in the south. Now I am sailing on the Gulf of Perl with my family, some of my friends and a few of my best warriors.]])),
   w=500,
   posy=1,
}

diary_page_2 = {
   title=_("The Rough Sea"),
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[It seems as if sailing on the Gulf of Perl was one of our biggest mistakes. Nature was against us and drove us into a dark, wild storm. I really don’t know how many hours have passed since the waves rose higher than our boat, but still it would be suicide to go outside.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[Our ship is badly damaged and is taking in more and more water. We can thank the Gods if we survive this black night with our lives.]])),
   w=400,
   posy=1,
}

diary_page_3 = {
   title=_("Survivors"),
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Finally, the Gods were with us!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[We landed on an unknown coast and found peaceful rest under the palm trees growing at the shore. But that is about as good as it gets. The truth is: when I woke up this morning, I saw nothing but sand around us.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[It really seems as if we have landed under the only palm trees existing in this far-away sandy desert. I fear we won’t find the help we need to get our ship repaired in good time.]])),
   w=400,
   posy=1,
}

saledus_1 = {
   title=_("A Foreboding"),
   body= saledus(_("Saledus looks around nervously…"),
      -- TRANSLATORS: Saledus
      _([[Sire, I fear we are not safe in this foreign land. Who knows what terrible creatures live beyond this forest, in that mighty desert? What if these creatures enter the woods and await the time to attack us?]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[Well, perhaps my fear is misguided, but it can’t be wrong to keep watch in the forests – so that we can be sure to see any potential enemies before they can see us.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[You really should build a blockhouse in the eastern portion of the forest. Then one of my men can keep watch in the darkness and keep us safe from these creatures.]]))
      .. new_objectives(obj_build_blockhouse),
   w=400,
}


saledus_2 = {
   title=_("Safe For Now"),
   body= saledus(_("Saledus speaks with a sigh of relief…"),
      -- TRANSLATORS: Saledus
      _([[Sire, I saw that the construction of the blockhouse was completed, so I have assigned one of my best soldiers to it to keep watch on the desert.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[This is a good step forward. Now we can feel a bit safer and can look forward to repairing our ship.]])),
   w=400,
}

amalea_1 = {
   title=_("Young Amalea"),
   body= amalea(_("Amalea smiles…"),
      -- TRANSLATORS: Amalea
      _([[Greetings, Lutius! I just met Saledus outside.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[He told me about the blockhouse. Well, I don’t think we need more blockhouses – instead, we might concentrate on other things now.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[It is absolutely clear that we need wood to repair our ship. So, I walked through the forest yesterday to look for a nice place for a lumberjack’s house and found one right south of our provisional headquarters.]]))
      .. new_objectives(obj_build_lumberjack),
   w=400,
}

amalea_2 = {
   title=_("Young Amalea"),
   body= amalea(_("Amalea recommends…"),
      -- TRANSLATORS: Amalea
      _([[I got the message that our first lumberjack has started his work today. Perhaps it would be a good idea to wait until he has cleared enough space for constructing two more lumberjack’s houses, so that we can harvest the logs faster.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But unfortunately the logs are useless for repairing our ship – we need to turn them into planks, which are needed by every ship and every bigger building too. So we should build a sawmill – as soon as we have enough space for this.]]))
      .. new_objectives(obj_build_sawmill_and_lumberjacks),
   w=400,
}

amalea_3 = {
   title=_("Young Amalea"),
   body= amalea(_("Amalea comes in…"),
      -- TRANSLATORS: Amalea
      _([[I’ve got two important things to talk about… First the good news:]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[I noticed that the construction of the sawmill is complete, so we can begin to refine the logs that the lumberjacks are harvesting into planks.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[But the bad news is that our lumberjacks harvest at an incredible speed. There are almost no trees left on this island.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[These trees provide shelter from the sandstorms that sweep in from the desert, and shade on the hot days, and they are the home of many gentle animals. We shouldn’t leave this island a complete desert.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Amalea
      _([[Lutius, please find someone who will take care of planting new trees.]]))
      .. new_objectives(obj_build_forester),
   w=400,
}

saledus_3 = {
   title=_("Thinking About the Future"),
   body= saledus(_("Saledus notes…"),
      -- TRANSLATORS: Saledus
      _([[Sire, I just thought about the rocks standing on the south shore of this land. Perhaps we could cut out some useful granite and beautiful marble.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[The repair of our ship will take a few weeks, anyway, and the resources we harvest now might be the base of strong and big buildings in another land.]]))
      .. new_objectives(obj_build_quarry),
   w=400,
}

saledus_4 = {
   title=_("A Step Forward"),
   body= saledus(_("Saledus looks excited…"),
      -- TRANSLATORS: Saledus
      _([[Sire, today we got a lot closer to our first castle. The quarry to the south began its work today and will soon provide us with granite and beautiful marble.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Saledus
      _([[Now we truly can look forward to settling down on another island.]])),
   w=400,
}

diary_page_4 = {
   title=_("Mission Complete"),
   body= lutius(_("Diary of Lutius"),
      -- TRANSLATORS: Lutius - Diary
      _([[Today I got the message that our ship is completely repaired. At the moment, my people are loading everything onto our newly repaired ship, hoping it will serve us better than last time.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[Tonight will be our last night on this island. Tomorrow morning we will leave, searching for a new place for our exile.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Lutius - Diary
      _([[I still wonder if we will ever see Fremil again.]]))
      .. objective_text(_("Victory"),
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   allow_next_scenario = true,
   posy=1
}

safe_peninsula = {
   title=_("Safe Peninsula"),
   body= saledus(_("Saledus speaks:"),
      -- TRANSLATORS: Saledus
      _([[Sire, you have made this peninsula a very secure place. Now it’s time to move on!]]))
}
