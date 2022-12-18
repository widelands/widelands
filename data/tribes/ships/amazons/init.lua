push_textdomain("tribes")

dirname = path.dirname(__file__)

wl.Descriptions():new_ship_type {
   name = "amazons_ship",
   -- TRANSLATORS: This is the ship's name used in lists of units
   descname = _("Ship"),
   capacity = 30,
   vision_range = 4,
   icon = dirname .. "menu.png",

   animation_directory = dirname,
   spritesheets = {
      idle = {
         hotspot = {73, 49},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
      sinking = {
         hotspot = {73, 49},
         fps = 7,
         frames = 21,
         columns = 3,
         rows = 7
      },
      sail = {
         directional = true,
         hotspot = {80, 52},
         fps = 15,
         frames = 30,
         columns = 6,
         rows = 5
      },
   },

   names = {
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Orinoco"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Amazonas"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Abacaxis"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Anchicayá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Guáitara"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Iscuandé"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Putumayo"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Acari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Acre"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Açuã"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Acurauá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Aiari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Alalaú"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Amanã"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Amapá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Andirá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Antimary"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Apoquitaua"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Apuaú"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Araçá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Arara"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Ararirá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Arauã"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Aripuanã"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Arrojo"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Atiparaná"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Atucatiquini"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Autaz-mirim"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Badajós"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Bararati"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Biá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Camaiú"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Camanaú"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Canumã"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Capucapu"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Carabinani"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Cauaburi"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Caurés"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Ciriquiri"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Coari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Copatana"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Coti"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Cubate"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Cuiari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Cuieiras"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Cuini"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Cuniuá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Curicuriari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Curiuaú"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Curuçá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Curuduri"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Curuquetê"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Daraá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Demini"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Eiru"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Endimari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Envira"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Gregório"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Guaribe"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Iá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Iaco"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Içá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Içana"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Igapó-Açu"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Inauini"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Ipixuna"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Itanhauá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Itaparaná"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Itaquai"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Itucumã"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Ituí"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Ituxi"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jacaré"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jamicia"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jandiatuba"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Japurá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jaquirana"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jatapu"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jatuarana"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jaú"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jauaperi"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Javary"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Juami"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jufari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Juma"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Juruá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Juruena"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jurupari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jutai"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Jutaizinho"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Luna"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Maiá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Maici"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Mamiá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Mamoriá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Mamuru"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Manacapuru"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Manicoré"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Manicorezinho"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Mapari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Mapiá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Maracanã"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Marari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Marauiá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Marié"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Mariepauá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Marimari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Mataurá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Matupiri"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Maués Açu"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Mineruázinho"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Muaco"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Mucum"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Mutum"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Nhamundá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Paciá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Padauari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Papagaio"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Papuri"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Paraconi"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Parauari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Pardo"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Pati"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Pauini"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Pinhuã"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Piorini"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Piratucu"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Pitinga"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Preto"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Puduari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Pureté"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Puruê"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Purus"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Quixito"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Sepatini"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Sepoti"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Seruini"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Sucunduri"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Tapajós"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Tapauá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Tarauacá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Tarumã Açu"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Tarumã Mirim"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Tea"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Tefé"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Tonantins"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Toototobi"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Traíra"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Tumiã"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Tupana"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Uaicurapa"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Uarini"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Uatumã"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Uaupés"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Umari"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Uneiuxi"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Unini"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Uruá"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Urubaxi"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Urubu"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Urucu"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Urupadi"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Xeruã"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Xie"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Apu Illapu"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Ayar Cachi"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Illapa"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Inti"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Kuychi"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Mama Killa"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Mama Occlo"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Manco Cápac"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Pachamama"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Quchamama"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Sachamama"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Viracocha"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca god
      pgettext("shipname", "Yakumama"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Atahuallpa"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Lloqe Yupanki"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Manqo Qhapaq"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Mayta Qhapaq"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Pachakuti"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Qhapaq Yupanki"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Thupa"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Urqon"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Washkar"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Wayna Qhapaq"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Yawar Waqaq"),
      -- TRANSLATORS: This Amazonian ship is named after an Inca ruler
      pgettext("shipname", "Zinchi Roqa"),
      -- TRANSLATORS: This Amazonian ship is named after the lake in the Andes
      pgettext("shipname", "Lake Titicaca"),
      -- TRANSLATORS: This Amazonian ship is named after the main Inca temple
      pgettext("shipname", "Koricancha"),
   },
   port_names = {
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Alter"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Anumā"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Aquiqui"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Arapixuna"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Aritapera"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Atamanai"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Amorim"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Canindé"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Cuia"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Cacoal"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Capichauā"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Cipoal"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Cupiranga"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Curariaca"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Curupari"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Curuá"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Diogo"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Guajará"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Guarana"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Gurupá"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Ipanema"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Ipaupixuna"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Irateua"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Itacoatiara"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Itanduba"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Itapari"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Jaburu"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Jacarecapá"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Jaguara"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Janauacá"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Macapá"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Maguari"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Mambeca"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Manacapuru"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Mararu"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Maripi"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Maruá"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Mataraí"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Miri"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Moju"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Mojui"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Morimutuba"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Oriximiná"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Pacoval"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Paranquara"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Paricó"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Pedreira"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Povoado"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Prainha"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Preta"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Santarém"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Socorro"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Solimōes"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Surucá"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Tambaqui"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Tapará"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Taperinha"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Tiningu"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Tomé"),
      -- TRANSLATORS: This Amazonian port is named after a town in the Amazon Basin
      pgettext("portname", "Una"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Bonampak"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Calakmul"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Chichén Itzá"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Copán"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Kaminaljuyú"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Lamanai"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Mayapán"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Pomoná"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Quiriguá"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Tikal"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Tononá"),
      -- TRANSLATORS: This Amazonian port is named after a Maya city
      pgettext("portname", "Uxmal"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Arequipa"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Aypate"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Cahuachi"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Choquequirao"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Kashamarka"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Kuelap"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Machu Picchu"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Nazca"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Ollantaytambo"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Pachacamac"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Paititi"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Paqari-tampu"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Cuzco"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Teyuna"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Tiahuanaco"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Vilcabamba"),
      -- TRANSLATORS: This Amazonian port is named after an Inca city
      pgettext("portname", "Vitcos"),
      -- TRANSLATORS: This Amazonian port is named after a part of the Inca Empire
      pgettext("portname", "Antisuyu"),
      -- TRANSLATORS: This Amazonian port is named after a part of the Inca Empire
      pgettext("portname", "Chinchasuyu"),
      -- TRANSLATORS: This Amazonian port is named after a part of the Inca Empire
      pgettext("portname", "Kuntisuyu"),
      -- TRANSLATORS: This Amazonian port is named after a part of the Inca Empire
      pgettext("portname", "Qullasuyu"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Achuar"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Aguaruna"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Aikanã"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Aimoré"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Amahuaca"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Amuesha"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Apiacá"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Arapaco"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Arara"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Ararandeura"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Arasairi"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Araueté"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Awá"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Aweti"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Banawá"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Baniwa"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Barasana"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Caeté"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Chaywita"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Cocama"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Enawene Nawe"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Goitacaz"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Himarimã"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Huambisa"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Irántxe"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Kachá"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Kagwahiva"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Kamayurá"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Kanamarí"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Kapixaná"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Kareneri"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Karitiâna"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Kaxinawá"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Kayapo"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Kobeua"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Korubu"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Kuikoro"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Machiguenga"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Majaguaje"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Makuna"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Marajoara"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Marubo"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Mayoruna"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Miriti"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Nahukuá"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Nambikwara"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Ocaina"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Omagua"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Pacanawa"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Pacawara"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Pirahã"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Potiguara"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Saraguro"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Sateré-Mawé"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Suruwaha"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Suyá"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Tabajara"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Tacana"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Tamoio"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Tapirapé"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Tariana"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Temiminó"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Terena"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Toromona"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Tremembé"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Tsimané"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Tupinambá"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Tupiniquim"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Uru-Eu-Wau-Wau"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Waorani"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Waurá"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Yaminawá"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Yaruna"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Yawalipiti"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Yawanawa"),
      -- TRANSLATORS: This Amazonian port is named after an indigenous people in the Amazon Basin
      pgettext("portname", "Yora"),
   }
}

pop_textdomain()
