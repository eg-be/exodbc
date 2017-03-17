DROP TABLE EXODBC.UTF8TABLE;

CREATE TABLE EXODBC.UTF8TABLE (
		IDUTF8TABLE INTEGER NOT NULL, 
		CONTENT VARCHAR(255)
	);
	
ALTER TABLE EXODBC.CHARTYPES ADD CONSTRAINT IDUTF8TABLE_PK PRIMARY KEY ( IDUTF8TABLE );		
	
INSERT INTO EXODBC.UTF8TABLE (IDUTF8TABLE, CONTENT) VALUES (1, 'После смерти отца оставил учёбу и поступил на службу газетным репортёром');
INSERT INTO EXODBC.UTF8TABLE (IDUTF8TABLE, CONTENT) VALUES (2, '因此他对世界的原型的认识就比较广泛。迁徙和旅行对他的创作很有助益');
INSERT INTO EXODBC.UTF8TABLE (IDUTF8TABLE, CONTENT) VALUES (3, 'Zürih üniversitesin de başladığı Alman Filolojisi eğitimini 1933 deki babasının ölümü üzerine');
INSERT INTO EXODBC.UTF8TABLE (IDUTF8TABLE, CONTENT) VALUES (4, 'מקס פריש נולד ב-15 במאי 1911 בציריך, בן לאדריכל פרנץ ברונו פריש ולאשתו קרולינה בטינה');
INSERT INTO EXODBC.UTF8TABLE (IDUTF8TABLE, CONTENT) VALUES (5, 'Στο έργο του ο Φρις διεισδύει, σχολιάζει και αναλύει τα διλήμματα του σύγχρονου ανθρώπου:');
