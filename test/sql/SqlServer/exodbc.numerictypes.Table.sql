USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[numerictypes]') AND type in (N'U'))
DROP TABLE [exodbc].[numerictypes]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[numerictypes]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[numerictypes](
	[idnumerictypes] [int] NOT NULL,
	[tdecimal_18_0] [decimal](18, 0) NULL,
	[tdecimal_18_10] [decimal](18, 10) NULL,
	[tdecimal_5_3] [decimal](5, 3) NULL,
 CONSTRAINT [PK_numerictypes] PRIMARY KEY CLUSTERED 
(
	[idnumerictypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (1, CAST(0 AS Decimal(18, 0)), NULL, NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (2, CAST(123456789012345678 AS Decimal(18, 0)), NULL, CAST(12.345 AS Decimal(5, 3)))
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (3, CAST(-123456789012345678 AS Decimal(18, 0)), NULL, NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (4, NULL, CAST(0.0000000000 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (5, NULL, CAST(12345678.9012345678 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (6, NULL, CAST(-12345678.9012345678 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (7, NULL, CAST(123.0567 AS Decimal(18, 10)), NULL)
