DROP TABLE IF EXISTS `unicodetable`;

CREATE TABLE `unicodetable` (
  `idunicodetable` int(11) NOT NULL,
  `content` varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,
  PRIMARY KEY (`idunicodetable`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `unicodetable` VALUES(1, 'После смерти отца оставил учёбу и поступил на службу газетным репортёром');
INSERT INTO `unicodetable` VALUES(2, '因此他对世界的原型的认识就比较广泛。迁徙和旅行对他的创作很有助益');
INSERT INTO `unicodetable` VALUES(3, 'Zürih üniversitesin de başladığı Alman Filolojisi eğitimini 1933 deki babasının ölümü üzerine');
INSERT INTO `unicodetable` VALUES(4, 'מקס פריש נולד ב-15 במאי 1911 בציריך, בן לאדריכל פרנץ ברונו פריש ולאשתו קרולינה בטינה');
INSERT INTO `unicodetable` VALUES(5, 'Στο έργο του ο Φρις διεισδύει, σχολιάζει και αναλύει τα διλήμματα του σύγχρονου ανθρώπου:');
