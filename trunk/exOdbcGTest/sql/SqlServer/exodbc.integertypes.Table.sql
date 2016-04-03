USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[integertypes]') AND type in (N'U'))
DROP TABLE [exodbc].[integertypes]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[integertypes]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[integertypes](
	[idintegertypes] [int] NOT NULL,
	[tsmallint] [smallint] NULL,
	[tint] [int] NULL,
	[tbigint] [bigint] NULL,
 CONSTRAINT [PK_integertypes] PRIMARY KEY CLUSTERED 
(
	[idintegertypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (1, -32768, NULL, NULL)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (2, 32767, NULL, NULL)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (3, NULL, -2147483648, NULL)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (4, NULL, 2147483647, NULL)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (5, NULL, NULL, -9223372036854775808)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (6, NULL, NULL, 9223372036854775807)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (7, -13, 26, 10502)
GO
