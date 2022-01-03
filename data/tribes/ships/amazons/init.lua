push_textdomain("tribes")

dirname = path.dirname(__file__)

descriptions:new_ship_type {
   name = "amazons_ship",
   -- TRANSLATORS: This is the ship's name used in lists of units
   descname = _"Ship",
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
      pgettext("shipname", "Dos Marmelos"),
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
      pgettext("shipname", "Liberdade"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Luna"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Madeira"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Madeirinha"),
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
      pgettext("shipname", "Mapiá Grande"),
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
      pgettext("shipname", "Preto da Eva"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Prêto do Igapó-Açu"),
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
      pgettext("shipname", "Rio Negro"),
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Riozinho"),
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
      -- TRANSLATORS: This Amazonian ship is named after a river
      pgettext("shipname", "Zinho"),
   },
}

pop_textdomain()
