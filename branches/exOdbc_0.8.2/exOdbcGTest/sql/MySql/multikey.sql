DROP TABLE IF EXISTS `multikey`;
CREATE TABLE `multikey` (
  `id1` int(11) NOT NULL,
  `id2` int(11) NOT NULL,
  `value` varchar(10) DEFAULT NULL,
  `id3` int(11) NOT NULL,
  PRIMARY KEY (`id1`,`id2`,`id3`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
