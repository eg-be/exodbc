USE [exodbc]
GO
/****** Object:  Table [exodbc].[numerictypes]    Script Date: 04.04.2015 17:12:41 ******/
DROP TABLE [exodbc].[numerictypes]
GO
/****** Object:  Table [exodbc].[numerictypes]    Script Date: 04.04.2015 17:12:41 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
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

GO
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (1, CAST(0 AS Decimal(18, 0)), NULL, NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (2, CAST(123456789012345678 AS Decimal(18, 0)), NULL, CAST(12.345 AS Decimal(5, 3)))
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (3, CAST(-123456789012345678 AS Decimal(18, 0)), NULL, NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (4, NULL, CAST(0.0000000000 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (5, NULL, CAST(12345678.9012345678 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (6, NULL, CAST(-12345678.9012345678 AS Decimal(18, 10)), NULL)
