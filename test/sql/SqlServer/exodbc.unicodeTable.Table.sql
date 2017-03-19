USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[unicodetable]') AND type in (N'U'))
DROP TABLE [exodbc].[unicodetable]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[unicodetable]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[unicodetable](
	[idunicodetable] [int] NOT NULL,
	[content] [nvarchar](255) NULL,
 CONSTRAINT [PK_unicodetable] PRIMARY KEY CLUSTERED 
(
	[idunicodetable] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_PADDING OFF
GO
INSERT [exodbc].[unicodetable] ([idunicodetable], [content]) VALUES (1, N'После смерти отца оставил учёбу и поступил на службу газетным репортёром')
INSERT [exodbc].[unicodetable] ([idunicodetable], [content]) VALUES (2, N'因此他对世界的原型的认识就比较广泛。迁徙和旅行对他的创作很有助益')
INSERT [exodbc].[unicodetable] ([idunicodetable], [content]) VALUES (3, N'Zürih üniversitesin de başladığı Alman Filolojisi eğitimini 1933 deki babasının ölümü üzerine')
INSERT [exodbc].[unicodetable] ([idunicodetable], [content]) VALUES (4, N'מקס פריש נולד ב-15 במאי 1911 בציריך, בן לאדריכל פרנץ ברונו פריש ולאשתו קרולינה בטינה')
INSERT [exodbc].[unicodetable] ([idunicodetable], [content]) VALUES (5, N'Στο έργο του ο Φρις διεισδύει, σχολιάζει και αναλύει τα διλήμματα του σύγχρονου ανθρώπου:')
