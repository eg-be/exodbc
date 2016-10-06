USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[integertypes_tmp]') AND type in (N'U'))
DROP TABLE [exodbc].[integertypes_tmp]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[integertypes_tmp]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[integertypes_tmp](
	[idintegertypes] [int] NOT NULL,
	[tsmallint] [smallint] NULL,
	[tint] [int] NULL,
	[tbigint] [bigint] NULL,
 CONSTRAINT [PK_integertypes_tmp] PRIMARY KEY CLUSTERED 
(
	[idintegertypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
INSERT [exodbc].[integertypes_tmp] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (1, -32768, -2147483648, -9223372036854775808)
INSERT [exodbc].[integertypes_tmp] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (2, 32767, 2147483647, 9223372036854775807)
