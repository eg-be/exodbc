DROP TABLE IF EXISTS `utf8table`;

CREATE TABLE `utf8table` (
  `idutf8table` int(11) NOT NULL,
  `content` varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,
  PRIMARY KEY (`idutf8table`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `utf8table` VALUES(1, 'После смерти отца оставил учёбу и поступил на службу газетным репортёром');
INSERT INTO `utf8table` VALUES(2, '因此他对世界的原型的认识就比较广泛。迁徙和旅行对他的创作很有助益');
INSERT INTO `utf8table` VALUES(3, 'Zürih üniversitesin de başladığı Alman Filolojisi eğitimini 1933 deki babasının ölümü üzerine');
INSERT INTO `utf8table` VALUES(4, 'מקס פריש נולד ב-15 במאי 1911 בציריך, בן לאדריכל פרנץ ברונו פריש ולאשתו קרולינה בטינה');
INSERT INTO `utf8table` VALUES(5, 'Στο έργο του ο Φρις διεισδύει, σχολιάζει και αναλύει τα διλήμματα του σύγχρονου ανθρώπου:');
