USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[integertable]') AND type in (N'U'))
DROP TABLE [exodbc].[integertable]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[integertable]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[integertable](
	[idintegertable] [int] NOT NULL,
	[tint1] [int] NULL,
	[tint2] [int] NULL,
	[tint3] [int] NULL,
 CONSTRAINT [PK_integertable] PRIMARY KEY CLUSTERED 
(
	[idintegertable] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
INSERT [exodbc].[integertable] ([idintegertable], [tint1], [tint2], [tint3]) VALUES (1, 2, 3, 4)
INSERT [exodbc].[integertable] ([idintegertable], [tint1], [tint2], [tint3]) VALUES (2, 20, NULL,40)
GO
