USE [exodbc]
GO
/****** Object:  Table [exodbc].[numerictypes_tmp]    Script Date: 04.04.2015 17:12:41 ******/
DROP TABLE [exodbc].[numerictypes_tmp]
GO
/****** Object:  Table [exodbc].[numerictypes_tmp]    Script Date: 04.04.2015 17:12:41 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [exodbc].[numerictypes_tmp](
	[idnumerictypes] [int] NOT NULL,
	[tdecimal_18_0] [decimal](18, 0) NULL,
	[tdecimal_18_10] [decimal](18, 10) NULL,
	[tdecimal_5_3] [decimal](5, 3) NULL,
 CONSTRAINT [PK_numerictypes_tmp] PRIMARY KEY CLUSTERED 
(
	[idnumerictypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
INSERT [exodbc].[numerictypes_tmp] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (1, CAST(-123456789012345678 AS Decimal(18, 0)), CAST(-12345678.9012345678 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes_tmp] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (2, CAST(123456789012345678 AS Decimal(18, 0)), CAST(12345678.9012345678 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes_tmp] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (3, CAST(0 AS Decimal(18, 0)), CAST(0.0000000000 AS Decimal(18, 10)), NULL)
