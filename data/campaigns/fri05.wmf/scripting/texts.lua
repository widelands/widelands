-- =========================
-- Some formating functions
-- =========================

include "scripting/richtext_scenarios.lua"

-- TODO(Nordfriese): Need new images for them all
function claus(title, text)
   return speech ("map:claus.png", styles.color("campaign_fri_claus"), title, text)
end
function henneke(title, text)
   return speech ("map:henneke.png", styles.color("campaign_fri_henneke"), title, text)
end
function reebaud(title, text)
   return speech ("map:reebaud.png", styles.color("campaign_fri_reebaud"), title, text)
end
function ketelsen(title, text)
   return speech ("map:ketelsen.png", styles.color("campaign_fri_ketelsen"), title, text)
end

-- =======================================================================
--                           Texts - No logic here
-- =======================================================================

local objective_message_height = messagebox_h_step(3)

-- ===========
-- objectives
-- ===========

local function warelist(t)
   local listitems = {}
   for ware,amount in pairs(t) do
      table.insert(listitems, _("%1$d× %2$s"):bformat(amount, game:get_ware_description(ware).descname))
   end
   return localize_list(listitems, "and")
end
local trade_text = ""
for i,trade_descr in ipairs(trade) do
   -- TRANSLATORS: The placeholders are ware lists in a trade description, e.g. "2× Fish and 1× Honey → 1× Granite"
   trade_text = trade_text .. li(_("%1$s → %2$s"):bformat(warelist(trade_descr[1]), warelist(trade_descr[2])))
end
obj_trade = {
   name = "trade",
   title =_("Trade"),
   number = 1,
   body = objective_text(_("Trade"),
      p(_([[You can not mine resources or quarry rocks on this island. Instead, you can trade local produce against other types of wares.]])) ..
      p(_([[Any wares stored in a port will be traded. The trade prices are indicated by the following chart.]]))
      .. trade_text
   ),
}

obj_fight = {
   name = "fight",
   title =_("Defeat the invaders"),
   number = 1,
   body = objective_text(_("Defeat Atterdag’s Forces."),
      li(_([[Destroy all the enemy’s buildings.]]))
   ),
}

obj_block = {
   name = "block",
   title =_("Block the bottleneck"),
   number = 1,
   body = objective_text(_("Block the Bottleneck."),
      li(_([[Secure the narrow passage to the east of the Uplands with several large military buildings, so the enemy cannot get past before you’re ready for the attack.]]))
   ),
}

obj_wood = {
   name = "wood",
   title =_("Wood production"),
   number = 1,
   body = objective_text(_("Secure your Wood Production."),
      li(_([[Build at least three woodcutter’s houses.]])) ..
      li(_([[Build at least four forester’s houses.]]))
   ),
}

obj_uplands = {
   name = "uplands",
   title =_("The Uplands"),
   number = 1,
   body = objective_text(_("Use the Uplands Wisely."),
      li(_([[Relocate your industry to the Uplands.]]))
   ),
}

-- ==================
-- Texts to the user
-- ==================

intro_1 = {
   title = _("Introduction"),
   body=claus(_("Welcome back!"),
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[Home at last! I cannot put into words the joy I feel at seeing my homelands again… nor the impatience at being so close and yet so far away.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 1
      _([[For once the sea was mild and permitted swift passage back South. Henneke and me both spent the longest part of the voyage standing at the ship’s bow looking forward to our return. Reebaud at first stood at the stern and looked back the same way as we did look ahead. Clearly he loves his new home as much as I do my old one. I am sure the wish to return alive and a victor will inspire his soldiers to fight as hard as my own troops will strive to reclaim our home.]]))
}

intro_2 = {
   title = _("Introduction"),
   body=claus(_("Welcome back!"),
      -- TRANSLATORS: Claus Lembeck – Introduction 2
      _([[The mood on our ship grew tenser as we approached familiar waters. The joyous laughter and excited anticipation of our return turned into a somber mood as we prepared for battle.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 2
      _([[And then the day arrived at last. We sighted the northernmost tip of the island Sylt and knew we’d land on Föhr on the evening of that same day. But then –]])),
}

intro_3 = {
   title = _("Introduction"),
   body=claus(_("Welcome back!"),
      -- TRANSLATORS: Claus Lembeck – Introduction 3
      _([[We had made one of the most basic mistakes, and we should all be ashamed of being no more careful than a cabin boy. Over the course of the months since we had left Föhr, we had neglected to pay attention to the calendar.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 3
      _([[Today is All Souls’ Day, the second day of November, and every child around here knows that no captain in his right mind would dream of setting sails within a week of this day. For time immemorial, the exceptionally strong storms that always blow at this time of year have sent many a careless sailor to a watery grave.]]))
}

intro_4 = {
   title = _("Introduction"),
   body=claus(_("Welcome back!"),
      -- TRANSLATORS: Claus Lembeck – Introduction 4
      _([[The sheerest luck alone saved us. Our ships were barely damaged, not one sailor lost to the waves. I would not believe that anyone could ever be so lucky had I not witnessed it myself. Our unbelievable escape fills me with hope – it is obviously a sign that our perilous endeavour is blessed by the saints.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 4
      _([[The storm drove our ship about two day’s journey from its course. Fortunately I still recognize these waters, so this is truly a very minor setback. When the storm subsided, we were close to the island of Helgoland and decided to land here. The islanders are well known for their hospitality, and the chance to rest and refresh ourselves before the battle will make a difference. Some of them might even join our cause.]]))
}

intro_5 = {
   title = _("Greetings"),
   body=ketelsen(_("Welcome to Helgoland!"),
      -- TRANSLATORS: Ocke Ketelsen – Introduction 5
      _([[Peace and welcome to Helgoland! Your reputation precedes you, Claus Lembeck, the well-loved lord of Föhr, and Reebaud whom the bards term the Bold and the Brave and the Mighty. I am Ocke Ketelsen, senior alderman of Helgoland, and despite these hard times I am pleased and honoured to receive such illustrious guests.]]))
}

intro_6 = {
   title = _("Greetings"),
   body=reebaud(_("Hospitality"),
      -- TRANSLATORS: Reebaud – Introduction 6
      _([[Peace and thanks for the friendly greeting to you, Ocke Ketelsen! It is a pleasant change not to be forced to go to war against someone on whose island we land. Yet your welcome sounds oddly short. Since your words are so warm, I can only conclude that some other sort of trouble has befallen you. Assuming you don’t mind if I ask and that this question does not appear impertinent, I would like to know if we can help you in any way.]]))
}

intro_7 = {
   title = _("Greetings"),
   body=ketelsen(_("Welcome to Helgoland!"),
      -- TRANSLATORS: Ocke Ketelsen – Introduction 7
      _([[You are remarkably astute. Indeed we have a problem, and you can see it right over there.]]))
}

intro_8 = {
   title = _("Greetings"),
   body=henneke(_("Enemies"),
      -- TRANSLATORS: Henneke Lembeck – Introduction 8
      _([[Danes! Does Atterdag’s thirst for conquest reach this far then?]]))
}

intro_9 = {
   title = _("Greetings"),
   body=ketelsen(_("Enemies"),
      -- TRANSLATORS: Ocke Ketelsen – Introduction 9
      _([[It does. I see that you recognize this enemy, and I would be interested in hearing how you met – though I imagine it will not have gone well for them!]]))
}

intro_10 = {
   title = _("Greetings"),
   body=claus(_("Enemies"),
      -- TRANSLATORS: Claus Lembeck – Introduction 10
      _([[This assumption is untrue, to my great chagrin. For the sake of brevity, I will only say that Atterdag invaded Föhr with such a mighty force that I was compelled to leave the island and ask Reebaud to help me reclaim it. In fact, we were sailing to Föhr to drive the invaders out once and for all when a storm blew us off our course and to Helgoland.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Introduction 10
      _([[That it did so might even be heavenly intervention. We will gladly help you fight this enemy. A warm-up battle, so to speak – I am looking forward to it!]]))
      .. new_objectives(obj_fight),
   h = objective_message_height,
}

trade_1 = {
   title = _("War Preparations"),
   body=ketelsen(_("Enemies"),
      -- TRANSLATORS: Ocke Ketelsen – Trade 1
      _([[Your enthusiasm warms me as much as your plight engages my sympathy. But the enemies are more powerful than they may appear – they have brought an enormous force of Danish soldiers, and even your mighty soldiers will not be able to defeat them all.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Ocke Ketelsen – Trade 1
      _([[Our island is not accustomed to this kind of warfare. We are fishers and farmers, we do not have the expertise or economy to support a drawn-out battle. Your expertise in this area would be much appreciated.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Ocke Ketelsen – Trade 1
      _([[But first let me give you a clear picture of our small island by showing you around.]]))
}

trade_2 = {
   title = _("War Preparations"),
   body=henneke(_("Economy"),
      -- TRANSLATORS: Henneke Lembeck – Trade 2
      _([[How do you even sustain yourselves here? These rocks look impressive but I don't see how you could quarry them to construct stone houses. I see hardly any trees, and no places where you could mine iron and other minerals! How do you come by?]]))
}

trade_3 = {
   title = _("War Preparations"),
   body=ketelsen(_("Trade"),
      -- TRANSLATORS: Ocke Ketelsen – Trade 3
      _([[By trade. We sell grain, fish, and various other goods to the mainland and buy ores and granite from there. Let’s split out duties according to what each of us does best: You take charge of reorganizing our economy for war and training our people for battle, and I will arrange to import goods in any quantity you require. Simply drop off the wares to sell in the ports and I will take care of the rest. This chart indicates how many goods of which sorts we usually trade.]]))
      .. new_objectives(obj_trade),
   h = objective_message_height,
}

getting_started_1 = {
   title = _("Planning the War"),
   body=reebaud(_("Use the Uplands wisely"),
      -- TRANSLATORS: Reebaud – Getting Started 1
      _([[First things first. This town has an excellent defensive position, but it is woefully unguarded. We must plan ahead tactically, because I agree that this battle will not be won quickly.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Getting Started 1
      _([[The Uplands have only one entrance: this ramp right here next to the town center. Let’s build all our important production buildings up there. A few fortresses placed closely together right at its southern tip will make this vast plateau nearly impossible to conquer. Even if the enemy should manage to take the entire lower island, we could conceivably hold out for years there, provided we stock enough supplies in warehouses on the Uplands in advance. Of course we’d be cut off from the ports and the trade routes, so we should try to avoid getting to this point.]]))
      .. new_objectives(obj_uplands),
   h = objective_message_height,
   position = "topleft",
}

getting_started_2 = {
   title = _("Planning the War"),
   body=reebaud(_("Fortify the narrow"),
      -- TRANSLATORS: Reebaud – Getting Started 2
      _([[Our most urgent concern, however, should be to secure this pass near our northernmost port. Our enemy must pass through this narrow point to reach us, but the same is also true in reverse. If we are careful, we can hold the enemy at this point for a long time until we are ready to launch our countercharge. However, the passage is too narrow to be fortified as easily as the Uplands. We must construct as many fortresses tightly together here as possible in order to prevent the enemy from gaining any land. It would be unwise to build anything at all north of the pass, as it would be dangerously exposed.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Getting Started 2
      _([[Depending on how many building materials we can spare, it might be wise to demolish some of the other buildings here to make room for even more fortifications. In any case, all buildings in this area, military or otherwise, will be in constant danger of being burnt down during a Danish incursion.]]))
      .. new_objectives(obj_block),
   h = objective_message_height,
   position = "topleft",
}

getting_started_3 = {
   title = _("Planning the War"),
   body=henneke(_("Wood supply"),
      -- TRANSLATORS: Henneke Lembeck – Getting Started 3
      _([[As Reebaud points out correctly, the Uplands are the best position for new buildings. Besides their strategic value they offer lots of space for construction, of which the lower island is somewhat short.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Henneke Lembeck – Getting Started 3
      _([[As for what we will build, I disagree. More military sites are important, but even more so is the ability to construct buildings in the first place. There are no woodcutters anywhere on the island, and we can’t rely on trade for wood, so we must secure our wood production at once before all our wood has been used up by the smokeries and charcoal burners!]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Henneke Lembeck – Getting Started 3
      _([[The land looks fairly barren, so we will need to reserve more space for foresting than I’m comfortable with. The Lowlands seem to be slightly more fertile, so it might be wise to build the foresters’ and woodcutters’ houses down here and relocate some of the farms to the Uplands. Farms, reed farms, and berry farms do not depend on fertile land quite as much as trees do.]]))
      .. new_objectives(obj_wood),
   h = objective_message_height,
   position = "topleft",
}

training = {
   title = _("Planning the War"),
   body=henneke(_("Long-term strategy"),
      -- TRANSLATORS: Henneke Lembeck – Training
      _([[Our trading has successfully started. Now we will need to plan our recruitment and training economy. We do have a basic food and metal production, but it seems to have grown haphazardly and will not suffice to meet our needs. We will require more numerous and more advanced sites to provide enough weapons and armour as well as food for our soldiers. Needless to say, a pair of training sites and a barracks are also essential.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Henneke Lembeck – Training
      _([[At all times, we will need to keep a close eye on our supply of tradeable goods. I will ask Ocke Ketelsen to purchase only the goods our economy actually needs so that we won’t waste any wares, but it will be up to us to ensure we can provide enough goods. These trade routes are all very expensive, and we will need to mark out a large amount of space purely to produce wares to sell.]])),
   position = "topleft",
}

diking = {
   title = _("Planning the War"),
   body=claus(_("Diking"),
      -- TRANSLATORS: Claus Lembeck – Diking
      _([[The lack of space on Helgoland is still a concern. The land would be sufficiently large to train a decent number of soldiers in a reasonable time, but training the armies we believe we will need would take forever! We may need to take some chances, and use superior tactics to compensate for this disadvantage.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Diking
      _([[However, if you prefer to play it safe, don’t forget that we can simply enlarge the island. As we have a steady supply of logs now, we can construct dikes and breakwaters to gain more building space from the sea. When we are ready to attack, we can even use this as a way to bypass the narrow point in the east, or if we are to be truly bold we could dike around the whole western coast of the island to attack the enemy from two sides at once!]])),
   position = "topleft",
}

getting_started_4 = {
   title = _("Planning the War"),
   body=ketelsen(_("Good luck!"),
      -- TRANSLATORS: Ocke Ketelsen – Getting Started 4
      _([[I am heartened by your forwardness. Let us all hasten to our separate tasks, and may fortune smile on us!]]))
}

secured_wood = {
   title = _("Wood Supply"),
   body=henneke(_("Wood supply is ensured"),
      -- TRANSLATORS: Henneke Lembeck – Wood Supply
      _([[Our foresters and woodcutters have taken up their work. It will take some time before we have a steady supply of logs, but the immediate problem has been averted.]]))
}

secured_block = {
   title = _("Pass Secured"),
   body=claus(_("The pass is under guard"),
      -- TRANSLATORS: Claus Lembeck – Pass Secured
      _([[Our soldiers have fortified the pass. Surely no enemy will get past them before we are ready.]]))
}

secured_uplands = {
   title = _("Uplands Colonized"),
   body=reebaud(_("The Uplands have been settled"),
      -- TRANSLATORS: Reebaud – Uplands Secured
      _([[The Uplands are being converted to a true center of our economy. We must not forget to always keep a close eye on the ware exchange between the Lowlands and the Uplands and keep it to a minimum so as to avoid jams on the roads.]]))
}

victory_1 = {
   title = _("Victory"),
   body=ketelsen(_("Enemy defeated"),
      -- TRANSLATORS: Ocke Ketelsen – Victory 1
      _([[They are defeated! Thank you, thank you for your help. We could never have beaten them by ourselves. Our town did not even sustain any major damage – on the contrary, you have expanded it far beyond its former bounds.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Ocke Ketelsen – Victory 1
      _([[My people and I will never be able to repay the debt we owe you. I will send all the soldiers you have trained along with you to aid you in your quest to liberate your own home. If there is anything else we can do for you, now or forevermore, you need only ask and we will be yours to command.]]))
}

victory_2 = {
   title = _("Victory"),
   body=claus(_("Enemy defeated"),
      -- TRANSLATORS: Claus Lembeck – Victory 2
      _([[You are very welcome, Ocke Ketelsen. Do not belittle your own part in this victory though, for we could not have achieved this by ourselves.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Claus Lembeck – Victory 2
      _([[And take my heartfelt thanks for your promise to send your soldiers. I already knew that Atterdag was a formidable enemy, but the forces we now encountered outrival anything I have seen in all my life, and they were just a vanguard! We will need every advantage we can get, and even then I am not as certain of victory as I was before. If you have heard of anything else that might be useful – any piece of tactical advice – any towns who might join us – please share this information with us.]]))
}

victory_3 = {
   title = _("Victory"),
   body=ketelsen(_("Know your enemy"),
      -- TRANSLATORS: Ocke Ketelsen – Victory 3
      _([[I will do so gladly. Indeed, according to my trade partners Atterdag’s thirst for conquest is limitless. After he has established his presence on Föhr, he has conquered all the other islands in the vicinity as well. Your stronghold on Föhr has always been their only defence to speak of, so once you were gone he seized the entire area with ease.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Ocke Ketelsen – Victory 3
      _([[The occupants are rumoured to enforce a cruel rule. It is a known fact that they have cut off all trade, and the locals are forced to give half of everything they produce to the Danes in tribute. Half! Already, dozens are said to have been executed for embezzling a bushel of grain. People are suffering, and will no doubt gladly join anyone who comes to free them.]]))
}

victory_4 = {
   title = _("Victory"),
   body=ketelsen(_("Know your enemy"),
      -- TRANSLATORS: Ocke Ketelsen – Victory 3
      _([[There is one more thing. Trade ships attempting to sail well north of the occupied area have been boarded and sunk by pirates. The survivors, however, swear that the pirates were in fact Danish soldiers, and that their ships belong to the Danish fleet.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Ocke Ketelsen – Victory 3
      _([[Now this is mainly speculation, but I believe that Atterdag transports his troops from the Danish capital to the islands he conquers by ship. The land route is much longer and more strenuous, so it is not hard to see why he would do that. Clearly he attempts to disguise this fact. Why would he do that, unless he believes this secret to be of strategical importance?]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Ocke Ketelsen – Victory 3
      _([[I do not know how large a part of his soldiers is exchanged between the capital and the islands by ship, but their numbers seem to be large enough that I would advise you to cut this exchange off. As you know, these two areas are connected by only one sea route, and it leads through a strait called the Kattegat. This strait is one a few miles wide and dotted with tiny islands. If you can reach this place without being attacked by their ships, you can seize the islands and build a belt of inconquerable strongholds that block any enemy ship attempting to pass.]]))
}

victory_5 = {
   title = _("Victory"),
   body=reebaud(_("Victory!"),
      -- TRANSLATORS: Reebaud – Victory 5
      _([[And if we can block it, they will lose their most important trade route! We must try it. We will sail to the Kattegat at once to conquer the strait, and then we’ll raise the people of Föhr and all the other islands in joint rebellion against the invader!]]))
}

victory_6 = {
   title = _("Victory"),
   body=ketelsen(_("Know your enemy"),
      -- TRANSLATORS: Ocke Ketelsen – Victory 6
      _([[I would like to make one more request. A request and an offer. My two sons, Harke Ockesen and Eschel Ockesen, are training as soldiers and commanders. I would much appreciate if if you would let them accompany you on your campaign to the Kattegat to gain some practical experience, and return them to me afterwards. In the meantime, we will use the industry you built for us to train more soldiers who shall aid you in your subsequent endeavours.]]))
}

victory_7 = {
   title = _("Victory"),
   body=reebaud(_("Victory!"),
      -- TRANSLATORS: Reebaud – Victory 7
      _([[We will be very glad to have them with us and I promise we will take good care of them. You certainly have a knack for assigning your sons the best teachers.]])
      .. paragraphdivider() ..
      -- TRANSLATORS: Reebaud – Victory 7
      _([[And thus we part, to meet again soon. Farewell, and may lasting peace bless all of Helgoland!]]))
      .. objective_text(_("Congratulations"),
      _([[You have completed this mission. You may continue playing if you wish, otherwise move on to the next mission.]])),
   allow_next_scenario = true,
}
