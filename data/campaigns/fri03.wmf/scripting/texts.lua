-- =========================
-- Some formating functions
-- =========================

include "scripting/richtext_scenarios.lua"

-- TODO(Nordfriese): Need new images for them all
function claus(title, text)
   return speech ("map:claus.png", "1CC200", title, text)
end
function henneke(title, text)
   return speech ("map:henneke.png", "F7FF00", title, text)
end
function iniucundus(title, text)
   return speech ("map:iniucundus.png", "FF0059", title, text)
end
function atlantean(title, text)
   return speech ("map:angadthur.png", "0016FF", title, text)
end

-- =======================================================================
--                           Texts - No logic here
-- =======================================================================

-- ===========
-- objectives
-- ===========

obj_north = {
   name = "north",
   title=_("Get to the North"),
   number = 1,
   body = objective_text(_("Go North"),
      li(_([[Sail to the North as far as possible, as fast as you can.]]))
   ),
}

obj_atl = {
   name = "atl",
   title=_("Destroy Angadthur’s Palace"),
   number = 1,
   body = objective_text(_("Destroy the Atlantean Headquarters and Build a Port"),
      li(_([[Build a port on the northern shore of the island. The port space is located under Angadthur’s Headquarters.]]))
   ),
}

function obj_emp(cost)
return {
   name = "emp",
   title=_("Pay or Fight"),
   number = 1,
   body = objective_text(_("Pay the Empire or Destroy Their Port"),
      li(ngettext([[Gather %d gold bar in your northernmost port to obtain a passage from Marcus Caius Iniucundus.]],
            [[Gather %d gold bars in your northernmost port to obtain a passage from Marcus Caius Iniucundus.]],
            cost):bformat(cost)) ..
      li(_([[Alternatively, you can destroy his port to build one of your own in it’s place.]])) ..
      li(_([[All soldiers who are present on the Northern island when you embark will accompany you.]])) ..
      li(_([[You have two hours to accomplish this objective.]]))
   ),
}
end

-- ==================
-- Texts to the user
-- ==================

intro_1 = {
   title = _("Introduction"),
   body=claus(_("Welcome back!"),
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[We have been sailing North for weeks until we reached this shore. Not one of my numerous seamaps gives me the information what lies beyond this point.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[Only one thing is certain: We must find a port space on the northern coast of this land belt to launch an expedition and continue sailing North.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[I want to find my old friend Reebaud as fast as I can. Every minute of idleness pains me as I imagine Atterdag gleefully trampling the green marshes of my Föhr under his dastard foot… Let’s make haste.]]))
      .. new_objectives(obj_north),
}

intro_2 = {
   title = _("Eight Weeks Ago"),
   body=claus(_("An unfair fight"),
      -- TRANSLATORS: Claus Lembeck – Introduction 2
      _([[But forgive me. I did not even introduce myself yet. I am Claus Lembeck, the knight who rules the island of Föhr… or I did until I was forced out by the conqueror.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 2
      _([[The island was prospering under my rule, and my subjects loved and respected me. They even built me a new castle as a gift. And then Atterdag came.]])),
}
intro_3 = {
   title = _("Eight Weeks Ago"),
   body=claus(_("An unfair fight"),
      -- TRANSLATORS: Claus Lembeck – Introduction 3
      _([[Atterdag! The King of the Danish Empire, and my sworn enemy. Many times did I aid the dukes of the mainland as they defended themselves against his thirst for war and conquest, and most times we fought him off. But eight weeks ago he landed with some of his allies and a large and well-trained army on my own island, Föhr, and sought to seize it.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 3
      _([[And I must speak the bitter admission – he succeeded. The fighting lasted five and thirty days, and every morning and evening yet another ship arrived from Denmark bringing in another troop of soldiers. How could I hope to compete against such odds? In the end I had no choice but to escape with my son Henneke and a few of my closest followers while Atterdag now calls Föhr another piece of his growing empire.]])),
}
intro_4 = {
   title = _("Eight Weeks Ago"),
   body=claus(_("An unfair fight"),
      -- TRANSLATORS: Claus Lembeck – Introduction 4
      _([[But I will not accept defeat!]])),
}
intro_5 = {
   title = _("Dark Future"),
   body=claus(_("Reconquest"),
      -- TRANSLATORS: Claus Lembeck – Introduction 5
      _([[It is evident that alone, I cannot hope to defeat this arch-enemy. Nor would those I fought alongside many a time be able to help me, seeing that we were always hard-pressed to repel Atterdag; and this latest assault of his proved that he was never even truly trying to win until now. He was merely toying with us. No, if I am to stand a chance of winning against him I need to find a greater force to aid my cause.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 5
      _([[And such a force it is I am seeking out now. Reebaud, the chieftain of a small but strong frisian community whom I once befriended; his haughty pride and his tremendous skill in battle are unmatched in all parts of the known world. If anyone can stand up to Atterdag, it must be him.]])),
}
intro_6 = {
   title = _("Dark Future"),
   body=claus(_("Reconquest"),
      -- TRANSLATORS: Claus Lembeck – Introduction 6
      _([[When I went to ask him for his help I was met with a sore disappointment though. He and his men no longer live in Northern Frisia but have chosen to found a new settlement in the far North where no men have ever dared to settle before. The bravery for so bold an undertaking is also something I would expect from nobody but him.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 6
      _([[It is a bravery we must attempt to copy, for there is but one course of action we can take. We must sail to the North and find my old friend Reebaud, and convince him to send soldiers with me to free my dear Föhr from the evil Dane.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 6
      _([[We must not fail. The future of my people depends on me.]])),
}

diker = {
   title = _("Diking"),
   body=claus(_("Gaining Land"),
      -- TRANSLATORS: Claus Lembeck – Diker
      _([[This island is a bit too small and cramped for my liking. There is an experienced diker among my people, and he has suggested to use some logs to construct breakwaters, which will allow us to seize some extra land from the sea. It will be a slow and expensive business, and I hope we will not stay on this island much, but at least we should consider it.]])),
}

port_1 = {
   title = _("Port Space Found"),
   body=claus(_("One step closer to the North"),
      -- TRANSLATORS: Claus Lembeck – Port Space 1
      _([[My soldiers report they have found a port space! Now we need to build a port and start an expedition. Of course, we’ll have to build a complete seafaring economy first, with a shipyard and a weaving mill. But don’t waste too much time perfecting our economy – all that matters is that we get to the North soon.]])),
}
port_2 = {
   title = _("A Strange Island"),
   body=claus(_("An apprehension"),
      -- TRANSLATORS: Claus Lembeck – Port Space 2
      _([[We have landed on a strange island. The ground is hard and black and steep, and the air smells burnt. The beach is the color of ashes, and the sand itself feels malevolent, as though a lindworm was cowering beneath it, waiting to pounce on any living creature foolish enough to come here. I can hardly put into words my feeling of apprehension. I do not know what there is to be afraid of, but we must be very cautious in any case as we cross it. Hopefully, we will reach the opposite side of the island soon.]]))
      .. new_objectives(obj_north),
}
atl_1 = {
   title = _("Strangers"),
   body=henneke(_("Who are they?"),
      -- TRANSLATORS: Henneke Lembeck – Strangers 1
      _([[Our soldiers have sighted strangers. Their buildings do not look like any I have seen before. I wonder who they are.]])),
}
atl_2 = {
   title = _("Strangers"),
   body=atlantean(_("Who are you?"),
      -- TRANSLATORS: Angadthur – Strangers 2
      _([[Be greeted, foreigners! I am Angadthur, the King of this miraculous island, a descendant of the legendary King Ajanthul the First of Atlantis himself. Be welcome to me, if you will be our friends!]])),
}
atl_3 = {
   title = _("Strangers"),
   body=claus(_("Greetings to the King!"),
      -- TRANSLATORS: Claus Lembeck – Strangers 3
      _([[Peace, King Angadthur! I am Claus Lembeck, and this is my son Henneke. We are searching for my old friend Reebaud, who has left our country several years ago to seek out a new home in the far North. If I may ask, have you heard of him before, and can you tell us where to find him?]])),
}
atl_4 = {
   title = _("Strangers"),
   body=atlantean(_("Greetings"),
      -- TRANSLATORS: Angadthur – Strangers 4
      _([[Of course you may ask. Ask whatever you like. Why do you ask whether you may ask?]])),
}
atl_5 = {
   title = _("Strangers"),
   body=claus(_("Greetings to the King!"),
      -- TRANSLATORS: Claus Lembeck – Strangers 5
      _([[It’s a figure of speech that is considered polite among our people, that’s all. Very well, I’ll ask properly: Do you know where we can find my friend Reebaud?]])),
}
atl_6 = {
   title = _("Strangers"),
   body=atlantean(_("Greetings"),
      -- TRANSLATORS: Angadthur – Strangers 6
      _([[I am sorry to disappoint you, Sir Lembeck, but I have never heard this name before and can therefore give you no news of him. As for your intent of travelling to the North – I am afraid there is but one suitable location for a port on this island other than the one where you did land, and it is located directly under my palace.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Angadthur – Strangers 6
      _([[As it so happens, my soldiers are bored with never having anything to do. Let us perform a contest of arms, shall we? You may send your soldiers to try and destroy my palace. I am certain that you will not stand a chance, and therefore I promise I will pray to the gods to bless your journey if you do succeed.]]))
      .. new_objectives(obj_atl),
}
atl_7 = {
   title = _("Congratulations"),
   body=atlantean(_("May Satul bless you"),
      -- TRANSLATORS: Angadthur – Atlantean Headquarter Destroyed
      _([[You have gotten past my best guards and burnt down my palace, which I believed to be impossible. You must be very powerful, and favoured by the gods. You have most certainly deserved your victory. I congratulate you and wish you well for your voyage. May Satul keep you safe!]])),
}

port_3 = {
   title = _("In the Ice Desert"),
   body=henneke(_("The next island"),
      -- TRANSLATORS: Henneke Lembeck – Port Space 3
      _([[We seem to be getting closer to the North. A desert of ice! I did not know such a thing even exists. But I doubt anybody could live here; I fear we will not discover anything useful on this island.]])),
}
port_4 = {
   title = _("In the Ice Desert"),
   body=claus(_("The next island"),
      -- TRANSLATORS: Claus Lembeck – Port Space 4
      _([[It doesn’t matter. Let’s just find the next port space quickly, so we can continue our journey.]]))
      .. new_objectives(obj_north),
}
port_5 = {
   title = _("In the Ice Desert"),
   body=claus(_("Port space found"),
      -- TRANSLATORS: Claus Lembeck – Port Space 5
      _([[We have found a port space on the island. Now we can leave this cold and bleak ice desert and continue sailing.]]))
      .. new_objectives(obj_north),
}
port_6 = {
   title = _("The North"),
   body=henneke(_("In the North"),
      -- TRANSLATORS: Henneke Lembeck – Port Space 6
      _([[Yes, this looks more like what I imagine the North should look like. The meadows are still green here, but the patches of snow are getting larger and larger the further North one can look.]])),
}
port_7 = {
   title = _("The North"),
   body=claus(_("In the North"),
      -- TRANSLATORS: Claus Lembeck – Port Space 7
      _([[It does look good, but it might be just another island. Let’s continue travelling quickly.]]))
      .. new_objectives(obj_north),
}

emp_1 = {
   title = _("Strangers"),
   body=henneke(_("Who are they?"),
      -- TRANSLATORS: Henneke Lembeck – Strangers 1
      _([[Our soldiers have sighted strangers again. I wonder whether they are similar to the strange King who congratulated us on destroying his palace.]])),
}
emp_2 = {
   title = _("Strangers"),
   body=iniucundus(_("No troubles, please"),
      -- TRANSLATORS: M. C. Iniucundus – Strangers 2
      _([[Greetings, Strangers! I am Marcus Caius Iniucundus, a legate of the Empire. Is this your island? We are only passing through, I hope you don’t mind. We don’t want any trouble.]])),
}
emp_3 = {
   title = _("Strangers"),
   body=claus(_("Travellers"),
      -- TRANSLATORS: Claus Lembeck – Strangers 3
      _([[Peace, Sir Iniucundus! I am Claus Lembeck, and this is my son Henneke. We are just passing through as well. You are a legate, you say? You must be getting around a lot. Have you heard news of my old friend Reebaud? I only know that he is living in the North, can you tell us where to find him?]])),
}
emp_4 = {
   title = _("Strangers"),
   body=iniucundus(_("Greetings"),
      -- TRANSLATORS: M. C. Iniucundus – Strangers 4
      _([[I am glad to hear you want peace. I am sorry, but I have not heard the name of Reebaud before. If you want to go to the North, however, we have the same destination, for that is exactly where I am travelling on official business from the Capital. We are currently preparing a ship to set sail. We can take you with us if you wish, provided you can pay for the passage.]])),
}
emp_5 = {
   title = _("Strangers"),
   body=claus(_("Travellers"),
      -- TRANSLATORS: Claus Lembeck – Strangers 5
      _([[We thank you for your offer! What is the price you demand? And, if you don’t mind my asking, may I know what business you are employed on?]])),
}
function emp_6(cost)
return {
   title = _("Strangers"),
   body=iniucundus(_("Greetings"),
      join_sentences(ngettext([[%d piece of gold.]], [[%d pieces of gold.]], cost):bformat(cost),
      -- TRANSLATORS: M. C. Iniucundus – Strangers 6
      _([[I know it seems much, but the Senate did not furnish me with much money for this expedition, so I must admit I am in need of some.]]))
      .. paragraphdivider() ..
      -- TRANSLATORS: M. C. Iniucundus – Strangers 6
      _([[The purpose of my journey is to seek out the Proconsule of the Province The Northern Lands, who has neglected to pay his taxes for several years running. My task is to give this Murilius a good telling-off, and if I discover he failed to pay out of greed rather than necessity, I am to bring him back to the Capital in chains. I must say, I am rather happy to be given this task, for I have a good chance of being appointed Proconsule in his stead if I succeed.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: M. C. Iniucundus – Strangers 6
      _([[But back to the topic of your passage. There is only one place on the northern coast of this island where a port can be built. So you do not really have an alternative to purchasing a passage on my galley. But my astrologers report that a terrible storm is gathering that will make sea travel impossible for a long time. We must set sail within two hours, and whoever is still on the island after that time will not leave it anytime soon.]])),
}
end
function emp_7(cost)
return {
   title = _("Strangers"),
   body=claus(_("Decide"),
      -- TRANSLATORS: Claus Lembeck – Strangers 7
      _([[He seems happy enough to offer us a passage, but the price is exorbitant. We may as well destroy his port and build one of our own. I wonder what would be better…]]))
      .. new_objectives(obj_emp(cost)),
}
end

victory_fight = {
   title =_("Victory"),
   body=claus(_("Victory!"),
      -- TRANSLATORS: Claus Lembeck – victory
      _([[We have conquered the port! We are already building a ship, and then, we will finally be travelling to the actual far North. Although we still heard no news of Reebaud, I am confident that we will find him soon. Then he will grant us aid, and I will reconquer Föhr from the usurper Atterdag!]]))
      .. objective_text(_("Congratulations"),
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   allow_next_scenario = true,
}
victory_pay = {
   title =_("Victory"),
   body=iniucundus(_("Victory!"),
      -- TRANSLATORS: M. C. Iniucundus – victory
      _([[You have paid for the passage with pure gold. Now it is only right that you get it. Our ship is ready, you may embark!]]))
      .. objective_text(_("Congratulations"),
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   allow_next_scenario = true,
}
timeout_1 = {
   title = _("You Have Lost"),
   body=iniucundus(_("Goodbye!"),
      -- TRANSLATORS: M. C. Iniucundus – victory
      _([[You have taken too long. We must set sail now, and then the storm will not permit you to leave this island. I am sorry for you, but the weather doesn’t wait. Goodbye.]]))
      .. objective_text(_("You Have Lost"),
      _([[You have lost and may not continue playing. May you have better luck when you retry this scenario. Click OK to return to the main menu.]])),
}
