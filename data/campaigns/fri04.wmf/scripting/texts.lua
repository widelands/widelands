-- =========================
-- Some formating functions
-- =========================

function claus(title, text)
   return speech ("map:claus.png", "1CC200", title, text)
end
function henneke(title, text)
   return speech ("map:henneke.png", "F7FF00", title, text)
end
function reebaud(title, text)
   return speech("map:reebaud.png", "55BB55", title, text)
end
function iniucundus(title, text)
   return speech ("map:iniucundus.png", "FF0059", title, text)
end
function amazon(title, text)
   return speech ("map:amazon.png", "6C0097", title, text)
end

-- =======================================================================
--                           Texts - No logic here
-- =======================================================================

-- ===========
-- objectives
-- ===========

obj_find_reebaud = {
   name = "find_reebaud",
   title=_("Find Reebaud"),
   number = 1,
   body = objective_text(_("Find Reebaud"),
      li(_([[Expand your territory until you meet your old friend Reebaud.]]))
   ),
}
obj_defeat_amz = {
   name = "defeat_amz",
   title=_("Defeat the strangers"),
   number = 1,
   body = objective_text(_("Defeat the Strangers"),
      li(_([[Defeat the hostile tribe.]]))
   ),
}
obj_defeat_emp = {
   name = "defeat_emp",
   title=_("Defeat the Empire"),
   number = 1,
   body = objective_text(_("Defeat the Imperial Legate"),
      li(_([[Defeat Marcus Caius Iniucundus.]]))
   ),
}
obj_recruit_train = {
   name = "recruit_train",
   title=_("Recruit & train"),
   number = 1,
   body = objective_text(_("Recruit And Train Your Soldiers"),
      li(_([[Train your soldiers while you are waiting for Reebaud to proclaim his soldiers battle-ready.]])) ..
      li(_([[You have one hour to train as many soldiers as you can.]]))
   ),
}

-- ==================
-- Texts to the user
-- ==================

intro_1 = {
   title =_("Welcome Back!"),
   body=claus(_("Arrived at last"),
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[Marcus Caius Iniucundus had not been lying about the storm. We had departed just in time; it caught us while we were on sea and we were all certain our ship was about to split and sink, and none of us would see our home again.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[But God was merciful, and our abundant prayers induced Him to spare us. We had had to land on a bleak, uninhabited shore and repair the ship ere we could continue sailing. Finally, we arrived as far North as it is possible to go by ship.]])),
}
intro_2 = {
   title =_("Welcome Back!"),
   body=claus(_("Arrived at last"),
      -- TRANSLATORS: Claus Lembeck – Introduction 2
      _([[The legate and I have no more business with each other now. I have taken a dislike to his superior attitude during the journey, and especially to the disdainful manner in which he treated his galley slaves. It did not seem to occur to him that they are human beings like him and me; or if it did, he acted as though they themselves were to blame for their bad fortune. I do not regret that I will probably not be seeing him again. He set up his camp some distance away from mine so that we will not hinder each other.]])),
}
intro_3 = {
   title =_("Welcome Back!"),
   body=claus(_("Arrived at last"),
      -- TRANSLATORS: Claus Lembeck – Introduction 3
      _([[But now that we have gotten so close, I am too impatient to sit around idly pondering the past. Quickly now! Let us go and look for my friend. Although I have seen no signs of human habitation around here yet, I have no doubt that we will find him soon.]]))
      .. new_objectives(obj_find_reebaud),
}

amz_1a = {
   title =_("Strangers"),
   body=henneke(_("Who are they?"),
      -- TRANSLATORS: Henneke Lembeck – Amazons 1a
      _([[Our soldiers have sighted strangers – the first natives in this bleak countryside. Perhaps they know where Reebaud can be found?]]))
}
amz_1b = {
   title =_("Strangers"),
   body=claus(_("Who are they?"),
      -- TRANSLATORS: Claus Lembeck – Amazons 1b
      _([[Peace and greetings, good people! I am Claus Lembeck, and this is my son Henneke. We come in search of my friend Reebaud. May I ask if you know where we may find him?]]))
}
amz_1c = {
   title =_("Strangers"),
   body=claus(_("Enemies"),
      -- TRANSLATORS: Claus Lembeck – Amazons 1c
      _([[Our soldiers have sighted strangers. These must be the intruders Reebaud mentioned.]]))
}
amz_2 = {
   title =_("Strangers"),
   body=amazon(_("Trespassers"),
      -- TRANSLATORS: Linthesilea – Amazons 2. She does not speak this language well, but she does manage to convey her meaning.
      _([[We – Amazons. You – strangers. Land – here – own – we. You – trespassers. You – enemies. We – you – all – make – dead!]]))
}
amz_3 = {
   title =_("Strangers"),
   body=henneke(_("Enemies"),
      -- TRANSLATORS: Henneke Lembeck – Amazons 3
      _([[Not the friendliest of people…]]))
      .. new_objectives(obj_defeat_amz),
}

reebaud_1 = {
   title =_("An Old Friend"),
   body=claus(_("Met up at last"),
      -- TRANSLATORS: Claus Lembeck – Reebaud 1
      _([[At last! These people’s buildings look familiar. It must be Reebaud’s tribe.]]))
}
reebaud_2 = {
   title =_("An Old Friend"),
   body=reebaud(_("Met up at last"),
      -- TRANSLATORS: Reebaud – Reebaud 2
      _([[Do my eyes deceive me, or is a spirit playing tricks on me? Surely it cannot be my old friend Claus who meets me so far away from home!]]))
}
reebaud_3 = {
   title =_("An Old Friend"),
   body=claus(_("Met up at last"),
      -- TRANSLATORS: Claus Lembeck – Reebaud 3
      _([[It is no trick or deception, it really is me, Claus Lembeck! My son Henneke and I journeyed this far to seek you out. I cannot say how glad I am to have found you at last.]]))
}
reebaud_4 = {
   title =_("An Old Friend"),
   body=reebaud(_("Met up at last"),
      -- TRANSLATORS: Reebaud – Reebaud 4
      _([[I am heartened that you went to such trouble for the sake of meeting me. But you did not do this just to talk to me again. What is it that troubles you? Just as you once helped me fend off a Holstein invasion, I will now help my friend if I can.]]))
}
reebaud_5 = {
   title =_("An Old Friend"),
   body=claus(_("Met up at last"),
      -- TRANSLATORS: Claus Lembeck – Reebaud 5
      _([[Indeed I came to ask for your help. Surely you remember Atterdag, the King of the Danes, who caused us Frisians great difficulties many a time. You never fought him yourself, he being too wary at your reputation to assault you directly; but I have made an arch-enemy of him by frequently aiding those he sought to conquer.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Reebaud 5
      _([[Some three or four months ago – I have lost track of time on the long voyage – he landed his soldiers on my island Föhr. I will spare you an account of the battle, which lasted five weeks and not a day less. The short and bitter truth is that I was defeated and forced to flee. I had no hope that my friends on the mainland could help me, for Atterdag’s armies are so incredibly numerous that I knew they wouldn’t have stood a chance.]]))
}
reebaud_6 = {
   title =_("An Old Friend"),
   body=claus(_("Met up at last"),
      -- TRANSLATORS: Claus Lembeck – Reebaud 6
      _([[Only you have a reputation for the most wonderous feats of arms. You are probably the only human alive who can vanquish the evil Dane. I beg you to come South with me and liberate Föhr, or at least to send with me a troop of your far-famed soldiers.]]))
}
reebaud_7 = {
   title =_("An Old Friend"),
   body=reebaud(_("Met up at last"),
      -- TRANSLATORS: Reebaud – Reebaud 7
      _([[Had you arrived a week earlier, I would have agreed to your request instantly. Now, however, a spot of bother has appeared on the horizon. I mean this quite literally; foreigners have come from far away and seek to drive us out and take over our lands as we ourselves did, driven by need, seize them from the hostile people who lived here. Warlike Barbarians, and an imperial scallywag named Murilius who made the most outrageous demands. We defeated them all, and believed to have found peace at last. And for four long years, life was peaceful indeed.]]))
}
reebaud_8a = {
   title =_("An Old Friend"),
   body=reebaud(_("Met up at last"),
      -- TRANSLATORS: Reebaud – Reebaud 8a
      _([[But now, as I was saying, in the arrival of the short period of summer, during which the snow melts in some places and travel to distant places is possible, the strangers appeared and engaged us in battle. They took us by surprise, the life without fighting having made us careless. Many a good soldier fell to their spears, including Hauke, my best friend and most venerable advisor, and Fiete, my only son and heir. Their sneaky battle moves were a novelty to us, though now that we know what we are up against there can be no doubt that we will beat them in the end. But I fear it will still take some time until they are defeated and it is safe for me to travel South with you. For this period, you shall of course receive all the hospitality and courtesy we can offer.]]))
}
reebaud_8b = {
   title =_("An Old Friend"),
   body=reebaud(_("Met up at last"),
      -- TRANSLATORS: Reebaud – Reebaud 8b
      _([[But now, as I was saying, in the arrival of the short period of summer, during which the snow melts in some places and travel to distant places is possible, the strangers appeared and engaged us in battle. They took us by surprise, the life without fighting having made us careless. Many a good soldier fell to their spears, including Hauke, my best friend and most venerable advisor, and Fiete, my only son and heir. Their sneaky battle moves were a novelty to us, though now that we knew what we were up against we managed to beat them in the end. However, all our soldiers are still sore and wounded from the heavy fighting, and they will need some time to recuperate before we can send them into battle again. For this period, you shall of course receive all the hospitality and courtesy we can offer.]]))
}
reebaud_9a = {
   title =_("An Old Friend"),
   body=claus(_("Met up at last"),
      -- TRANSLATORS: Claus Lembeck – Reebaud 9a
      _([[I believe we have already met these intruders. It would be unseemly to sit around enjoying mugs of mead while you are fighting for your lives. In the spirit of our friendship, and in honour of the good men you lost, we will fight the invaders alongside you!]]))
}
reebaud_9b = {
   title =_("An Old Friend"),
   body=claus(_("Met up at last"),
      -- TRANSLATORS: Claus Lembeck – Reebaud 9b
      _([[Truly, it would be most unseemly to sit around enjoying mugs of mead while you are fighting for your lives. In the spirit of our friendship, and in honour of the good men you lost, we shall seek out those invaders and fight them alongside you!]]))
      .. new_objectives(obj_defeat_amz),
}
reebaud_9c = {
   title =_("An Old Friend"),
   body=henneke(_("Met up at last"),
      -- TRANSLATORS: Henneke Lembeck – Reebaud 9c
      _([[I am almost disappointed that your reputation does not exceed the facts, for as you where describing the invaders I was looking forward to fighting them alongside you. I am sorry for your losses, and especially that I will never meet the renowned Hauke, whose name will live on in fame. While your soldiers are recuperating, I will take charge of arranging training sessions for our own soldiers, so that they may learn from yours and become as strong as possible before we return to our homeland.]]))
      .. new_objectives(obj_recruit_train),
}

reebaud_10 = {
   title =_("An Old Enemy"),
   body=iniucundus(_("Treason"),
      -- TRANSLATORS: Marcus Caius Iniucundus – Reebaud 10
      _([[Wait a minute, you two. What was this you said? You assaulted and murdered Murilius, a Proconsule of the Empire?]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Marcus Caius Iniucundus – Reebaud 10
      _([[This is most inacceptable. I am Marcus Caius Iniucundus, a legate of the Empire. To think that I was sent to investigate whether Murilius had neglected to pay his taxes from need or greed, and that I now discovered that he and all his men were foully murdered by arrogant invaders!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Marcus Caius Iniucundus – Reebaud 10
      _([[By your deed, you have bestowed upon yourself the wrath of the Empire, and I swear by Jupiter Optimus Maximus that I will not rest until you and yours are lying in your own blood. I will be merciful only to those too young to have participated in the fighting then, whom I will keep alive to replace the handful of galley slaves who did not survive the latest journey.]]))
}
reebaud_11 = {
   title =_("An Old Enemy"),
   body=reebaud(_("Treason"),
      -- TRANSLATORS: Reebaud – Reebaud 11
      _([[Oh, now I’m scared. Tell me, good man, is foolishness a requirement to be appointed a representative of the Empire? Claus, what did you bring that deplorable jester with you for?]]))
}
reebaud_12 = {
   title =_("An Old Enemy"),
   body=claus(_("Treason"),
      -- TRANSLATORS: Claus Lembeck – Reebaud 12
      _([[I am sorry. He offered us a passage on his ship, and I had no way of knowing nor any reason to assume that he would seek a quarrel with you. But seeing that it is I who brought this trouble to you, the least I can do is to take responsibility for ridding you of him.]]))
      .. new_objectives(obj_defeat_emp),
}

reebaud_in_danger = {
   title =_("Strangers"),
   body=henneke(_("Something is going on…"),
      -- TRANSLATORS: Henneke Lembeck – Reebaud in danger
      _([[We met a wounded soldier near our north-western border today. He gasped something about his tribe being attacked by vicous strangers, but he died before he could tell us more. His armour and weaponry looked startlingly familiar. I wonder if Reebaud’s tribe is in danger?]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Henneke Lembeck – Reebaud in danger
      _([[We must hurry.]])),
}

reebaud_defeated_1 = {
   title =_("Strangers"),
   body=henneke(_("We are too late"),
      -- TRANSLATORS: Henneke Lembeck – Reebaud defeated 1
      _([[We met a wounded soldier near our north-western border today. He gasped something about his tribe being attacked and vanquished by vicous strangers. With his last breath he told us his leader’s name: Reebaud.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Henneke Lembeck – Reebaud defeated 1
      _([[We are too late. Reebaud was already in our reach, and he was defeated mere hours before we could meet him. There is no hope in this world for any of us.]]))
      .. objective_text(_("You Have Lost"),
      _([[You have lost and may not continue playing. May you have better luck when you retry this scenario. Click OK to return to the main menu.]])),
}

reebaud_defeated_2 = {
   title =_("Defeat"),
   body=reebaud(_("We have lost"),
      -- TRANSLATORS: Reebaud – Reebaud defeated 2
      _([[The enemies have destroyed our last warehouse. No tribe could survive under these odds, and all we can do now is to face our certain fate with our heads held high. My only regret is that I will not be able to help my old friend Claus in his predicament. May the gods take pity on him, but I fear that all hope is lost.]]))
      .. objective_text(_("You Have Lost"),
      _([[You have lost and may not continue playing. May you have better luck when you retry this scenario. Click OK to return to the main menu.]])),
}

player_defeated = {
   title =_("Defeat"),
   body=claus(_("We have lost"),
      -- TRANSLATORS: Claus Lembeck – Player defeated
      _([[We were so close, but now the enemies have destroyed our last warehouse. Maybe it was foolish of me to assume I would be able to defeat Atterdag if I was not even a match for these insignificant foreigners. God disapproves of this my hubris, and He, not some random gaggle of strangers, has finalized our sad destiny.]]))
      .. objective_text(_("You Have Lost"),
      _([[You have lost and may not continue playing. May you have better luck when you retry this scenario. Click OK to return to the main menu.]])),
}

legate_expands = {
   title =_("Expansion"),
   body=henneke(_("We are being cut off"),
      -- TRANSLATORS: Henneke Lembeck – Empire expands
      _([[Marcus Caius Iniucundus is expanding his territory quickly. While he is perfectly within his rights to do so, I am worried that we might be cut off from Reebaud’s land if we don’t watch out.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Henneke Lembeck – Empire expands
      _([[We should increase the pace of our own expansion. Without knowing in which direction we need to head, I’d advise to focus on spreading westwards for now before Marcus Caius Iniucundus blocks us out on this frontier.]])),
}

victory_amz_a = {
   title =_("Victory"),
   body=claus(_("Intruders defeated"),
      -- TRANSLATORS: Claus Lembeck – Victory over Amazons
      _([[These intruders were not as strong as they thought. They will trouble you no more.]])),
}
victory_amz_b = {
   title =_("Victory"),
   body=claus(_("Strangers defeated"),
      -- TRANSLATORS: Claus Lembeck – Victory over Amazons
      _([[These strangers were not as strong as they thought. Whoever they were, they will trouble us no more.]])),
}
victory_emp = {
   title =_("Victory"),
   body=reebaud(_("Legate defeated"),
      -- TRANSLATORS: Reebaud – Victory over Empire
      _([[Attempting to take me on is the last mistake you ever made, foolish legate.]])),
}
victory_a = {
   title =_("Victory"),
   body=reebaud(_("We have won"),
      -- TRANSLATORS: Reebaud – Victory
      _([[The enemies are defeated. Now I will come South with you together with all my soldiers except for a small garrison to safeguard my lands, and we will reconquer your island!]]))
      .. objective_text(_("Congratulations"),
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   allow_next_scenario = true,
}
victory_b = {
   title =_("Victory"),
   body=reebaud(_("We have won"),
      -- TRANSLATORS: Reebaud – Victory
      _([[Our soldiers have recovered, and you have spent the time well by increasing the strength of your army. Now I will come South with you together with all my soldiers except for a small garrison to safeguard my lands, and we will reconquer your island!]]))
      .. objective_text(_("Congratulations"),
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   allow_next_scenario = true,
}
