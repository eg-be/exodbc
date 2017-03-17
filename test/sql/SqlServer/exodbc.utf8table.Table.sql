USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[utf8table]') AND type in (N'U'))
DROP TABLE [exodbc].[utf8table]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[utf8table]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[utf8table](
	[idutf8table] [int] NOT NULL,
	[content] [varbinary](255) NULL,
 CONSTRAINT [PK_utf8table] PRIMARY KEY CLUSTERED 
(
	[idutf8table] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_PADDING OFF
GO
INSERT [exodbc].[utf8table] ([idutf8table], [content]) VALUES (1, CONVERT(VARBINARY(255), 'После смерти отца оставил учёбу и поступил на службу газетным репортёром'))
INSERT [exodbc].[utf8table] ([idutf8table], [content]) VALUES (2, CONVERT(VARBINARY(255), '因此他对世界的原型的认识就比较广泛。迁徙和旅行对他的创作很有助益'))
INSERT [exodbc].[utf8table] ([idutf8table], [content]) VALUES (3, CONVERT(VARBINARY(255), 'Zürih üniversitesin de başladığı Alman Filolojisi eğitimini 1933 deki babasının ölümü üzerine'))
INSERT [exodbc].[utf8table] ([idutf8table], [content]) VALUES (4, CONVERT(VARBINARY(255), 'מקס פריש נולד ב-15 במאי 1911 בציריך, בן לאדריכל פרנץ ברונו פריש ולאשתו קרולינה בטינה'))
INSERT [exodbc].[utf8table] ([idutf8table], [content]) VALUES (5, CONVERT(VARBINARY(255), 'Στο έργο του ο Φρις διεισδύει, σχολιάζει και αναλύει τα διλήμματα του σύγχρονου ανθρώπου:'))
