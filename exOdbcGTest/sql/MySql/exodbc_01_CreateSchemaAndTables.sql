SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL,ALLOW_INVALID_DATES';

CREATE SCHEMA IF NOT EXISTS `exodbc` DEFAULT CHARACTER SET utf8 ;
USE `exodbc` ;

-- -----------------------------------------------------
-- Table `exodbc`.`blobtypes`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`blobtypes` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`blobtypes` (
  `idblobtypes` INT(11) NOT NULL,
  `tblob` BINARY(16) NULL DEFAULT NULL,
  `tvarblob_20` VARBINARY(20) NULL DEFAULT NULL,
  PRIMARY KEY (`idblobtypes`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`blobtypes_tmp`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`blobtypes_tmp` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`blobtypes_tmp` (
  `idblobtypes` INT(11) NOT NULL,
  `tblob` BINARY(16) NULL DEFAULT NULL,
  `tvarblob_20` VARBINARY(20) NULL DEFAULT NULL,
  PRIMARY KEY (`idblobtypes`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`chartable`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`chartable` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`chartable` (
  `idchartable` INT(11) NOT NULL,
  `col2` VARCHAR(128) NULL DEFAULT NULL,
  `col3` VARCHAR(128) NULL DEFAULT NULL,
  `col4` VARCHAR(128) NULL DEFAULT NULL,
  PRIMARY KEY (`idchartable`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`chartypes`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`chartypes` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`chartypes` (
  `idchartypes` INT(11) NOT NULL,
  `tvarchar` VARCHAR(128) NULL DEFAULT NULL,
  `tchar` CHAR(128) NULL DEFAULT NULL,
  `tvarchar_10` VARCHAR(10) NULL DEFAULT NULL,
  `tchar_10` CHAR(10) NULL DEFAULT NULL,
  PRIMARY KEY (`idchartypes`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`chartypes_tmp`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`chartypes_tmp` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`chartypes_tmp` (
  `idchartypes` INT(11) NOT NULL,
  `tvarchar` VARCHAR(128) NULL DEFAULT NULL,
  `tchar` VARCHAR(128) NULL DEFAULT NULL,
  `tvarchar_10` VARCHAR(10) NULL DEFAULT NULL,
  `tchar_10` CHAR(10) NULL DEFAULT NULL,
  PRIMARY KEY (`idchartypes`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`datetypes`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`datetypes` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`datetypes` (
  `iddatetypes` INT(11) NOT NULL,
  `tdate` DATE NULL DEFAULT NULL,
  `ttime` TIME NULL DEFAULT NULL,
  `ttimestamp` TIMESTAMP NULL DEFAULT NULL,
  PRIMARY KEY (`iddatetypes`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`datetypes_tmp`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`datetypes_tmp` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`datetypes_tmp` (
  `iddatetypes` INT(11) NOT NULL,
  `tdate` DATE NULL DEFAULT NULL,
  `ttime` TIME NULL DEFAULT NULL,
  `ttimestamp` TIMESTAMP NULL DEFAULT NULL,
  PRIMARY KEY (`iddatetypes`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`floattypes`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`floattypes` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`floattypes` (
  `idfloattypes` INT(11) NOT NULL,
  `tdouble` DOUBLE NULL DEFAULT NULL,
  `tfloat` FLOAT NULL DEFAULT NULL,
  PRIMARY KEY (`idfloattypes`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`floattypes_tmp`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`floattypes_tmp` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`floattypes_tmp` (
  `idfloattypes` INT(11) NOT NULL,
  `tdouble` DOUBLE NULL DEFAULT NULL,
  `tfloat` FLOAT NULL DEFAULT NULL,
  PRIMARY KEY (`idfloattypes`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`integertypes`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`integertypes` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`integertypes` (
  `idintegertypes` INT(11) NOT NULL,
  `tsmallint` SMALLINT(6) NULL DEFAULT NULL,
  `tint` INT(11) NULL DEFAULT NULL,
  `tbigint` BIGINT(20) NULL DEFAULT NULL,
  PRIMARY KEY (`idintegertypes`),
  UNIQUE INDEX `idQueryTypes_UNIQUE` (`idintegertypes` ASC))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`integertypes_tmp`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`integertypes_tmp` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`integertypes_tmp` (
  `idintegertypes` INT(11) NOT NULL,
  `tsmallint` SMALLINT(6) NULL DEFAULT NULL,
  `tint` INT(11) NULL DEFAULT NULL,
  `tbigint` BIGINT(20) NULL DEFAULT NULL,
  PRIMARY KEY (`idintegertypes`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`multikey`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`multikey` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`multikey` (
  `id1` INT(11) NOT NULL,
  `id2` INT(11) NOT NULL,
  `value` VARCHAR(10) NULL DEFAULT NULL,
  `id3` INT(11) NOT NULL,
  PRIMARY KEY (`id1`, `id2`, `id3`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`numerictypes`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`numerictypes` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`numerictypes` (
  `idnumerictypes` INT(11) NOT NULL,
  `tdecimal_18_0` DECIMAL(18,0) NULL DEFAULT NULL,
  `tdecimal_18_10` DECIMAL(18,10) NULL DEFAULT NULL,
  `tdecimal_5_3` DECIMAL(5,3) NULL DEFAULT NULL,
  PRIMARY KEY (`idnumerictypes`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


-- -----------------------------------------------------
-- Table `exodbc`.`numerictypes_tmp`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `exodbc`.`numerictypes_tmp` ;

CREATE TABLE IF NOT EXISTS `exodbc`.`numerictypes_tmp` (
  `idnumerictypes` INT(11) NOT NULL,
  `tdecimal_18_0` DECIMAL(18,0) NULL DEFAULT NULL,
  `tdecimal_18_10` DECIMAL(18,10) NULL DEFAULT NULL,
  `tdecimal_5_3` DECIMAL(5,3) NULL DEFAULT NULL,
  PRIMARY KEY (`idnumerictypes`))
ENGINE = InnoDB
DEFAULT CHARACTER SET = utf8;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
