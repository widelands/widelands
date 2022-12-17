push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ship_type {
   name = "empire_ship",
   -- TRANSLATORS: This is the Empire's ship's name used in lists of units
   descname = pgettext("empire_ship", "Ship"),
   animation_directory = dirname,
   icon = dirname .. "menu.png",
   capacity = 30,
   vision_range = 4,

   spritesheets = {
      idle = {
         fps = 10,
         frames = 40,
         rows = 7,
         columns = 6,
         hotspot = { 88, 90 }
      },
      sail = {
         fps = 10,
         frames = 40,
         rows = 7,
         columns = 6,
         directional = true,
         hotspot = { 111, 92 }
      },
   },

   names = {
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Adriatic Sea"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Adamas"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Agrippa"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Alexandria"),
      -- TRANSLATORS: This Empire ship is named after an in-game character
      pgettext("shipname", "Amalea"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Amolius"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Apollo"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Aquila"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Augustus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Aurum"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Azurea Sea"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Baltic Sea"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Bellona"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Brutus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Caligula"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Camilla"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Carrara"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Cato"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Cæsar"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Cervus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Cicero"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Claudius"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Diana"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Delphinus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Domitianus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Etna"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Faustulus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Ferrum"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Galba"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Gallus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Gladiator"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Gloria Imperii"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Ianus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Iulius Cæsar"),
      -- TRANSLATORS: This Empire ship is named after a Roman goddess
      pgettext("shipname", "Iuno"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Iupiter"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Latium"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Latonia"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Leo"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Lepus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Liguria"),
      -- TRANSLATORS: This Empire ship is named after an in-game character
      pgettext("shipname", "Lutius"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Marble"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Marcus Aurelius"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Mars"),
      -- TRANSLATORS: This Empire ship is named after a Roman goddess
      pgettext("shipname", "Minerva"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Neptune"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Nero"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Nerva"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Octavianus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Otho"),
      -- TRANSLATORS: This Empire ship is named after a Roman god
      pgettext("shipname", "Pluto"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Portus Bellus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Præda"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Quirinus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Remus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Romulus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Sagitta"),
      -- TRANSLATORS: This Empire ship is named after an in-game character
      pgettext("shipname", "Saledus"),
      -- TRANSLATORS: This Empire ship is named after a Roman deity
      pgettext("shipname", "Saturn"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Scipio"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Seneca"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Siena"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Spiculum"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Thyrrenia"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Tiberius"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Titus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Traianus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Venator"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Venus"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Vergilius"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Vespa"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Vespasianus"),
      -- TRANSLATORS: This Empire ship is named after a Roman goddess
      pgettext("shipname", "Vesta"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Vesuvius"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Vinea"),
      -- TRANSLATORS: This is an Empire ship name
      pgettext("shipname", "Vitellius"),
   },
   port_names = {
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Abila"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Ad Flexum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aelia Capitolina"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aeminium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aequum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Ala"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Albanianis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Ambianum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Andautonia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Apulum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aquæ Granni"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aquæ Helveticæ"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aquæ Mattiacorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aquæ Sulis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aquileia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aquincum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aræ Flaviæ"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Argentoratum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Ariminum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Ascrivium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Asturica Augusta"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Atuatuca Tungrorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Augusta Raurica"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Augusta Taurinorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Augusta Treverorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Augusta Vindelicorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aurelia Aquensis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Aventicum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Belum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Biriciana"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Blestium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Bonames"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Bonna"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Bononia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Borbetomagus"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Bovium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Bracara Augusta"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Brigantium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Burgodunum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Cæsaraugusta"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Cæsarea"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Cæsaromagus"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Calisia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Calleva Atrebatum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Camulodunum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Carnuntum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Castra Batavar"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Castra Legionis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Castra Regina"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Castra Vetera"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Cibalæ"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Clausentum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Colonia Agrippina"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Colonia Narbo Martius"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Colonia Ulpia Traiana"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Comagenis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Complutum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Concangis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Condate"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Confloenta"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Constantinopolis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Coria"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Corinium Dobunnorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Coriovallum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Cosa"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Curia Rætorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Danum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Deva Victrix"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Divodurum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Dubris"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Durnovaria"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Durocobrivis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Durocornovium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Duroliponte"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Durovigutum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Eboracum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Emerita Augusta"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Florentia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Forum Hadriani"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Forum Livii"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Gerulata"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Gerunda"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Gesoriacum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Gorsium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Hadrianopolis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Isca Augusta"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Isca Dumnoniorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Italica"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Iulia Aemona"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Iulia Romula Hispalis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Iuvavum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lactodurum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lagentium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lauri"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lentia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Letocetum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lindinis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lindum Colonia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Londinium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lopodunum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lousonna"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lucus Augusta"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lugdunum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lugdunum Batavorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Luguvalium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Lutetia Parisiorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Mamucium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Marsonia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Massa"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Matilo"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Mediolanum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Modoetia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Mogontiacum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Moridunum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Mursa"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Naissus"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Nemausus"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Nicopolis ad Istrum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Nicopolis ad Nestum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Nida"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Nigrum Pullum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Norba Cæsarina"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Novæ"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Novæsium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Noviodunum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Noviomagus"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Olicana"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Ovilava"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Palma"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Partiscum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Pistoria"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Placentia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Poetovium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Pomaria"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Pompælo"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Pompeii"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Pons Aelius"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Pons Drusi"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Portus Lemanis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Prætorium Agrippinæ"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Ragusium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Ratæ Corieltauvorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Regium Lepidi"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Regulbium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Rigomagus"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Roma"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Rutupiæ"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Salernum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Salona"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Savaria"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Scalabis"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Singidunum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Sirmium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Siscia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Sopiane"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Sorviodurum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Sostra"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Storgosia"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Sumelocenna"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Tarraco"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Theranda"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Tibiscum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Traiectum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Traiectum ad Mosam"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Traiectum ad Nicrem"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Turicum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Urbs Aurelianorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Valentia Edetanorum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Venetiæ"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Venta Belgarum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Venta Silurum"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Verulamium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Vesontio"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Viminacium"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Vindobona"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Vindonissa"),
      -- TRANSLATORS: This Empire port is named after a Roman town
      pgettext("portname", "Vinovia"),
   }
}

pop_textdomain()
