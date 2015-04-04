USE [exodbc]
GO
DROP TABLE [exodbc].[chartypes]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [exodbc].[chartypes](
	[idchartypes] [int] NOT NULL,
	[tvarchar] [varchar](128) NULL,
	[tchar] [char](128) NULL,
	[tvarchar_10] [nvarchar](10) NULL,
	[tchar_10] [nchar](10) NULL,
 CONSTRAINT [PK_chartypes] PRIMARY KEY CLUSTERED 
(
	[idchartypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (1, N' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~', NULL, NULL, NULL)
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (2, NULL, N' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ', NULL, NULL)
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (3, N'äöüàéè', NULL, NULL, NULL)
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (4, NULL, N'äöüàéè                                                                                                                          ', NULL, NULL)
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (5, NULL, NULL, N'abc', N'abc       ')
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (6, NULL, NULL, N'abcde12345', N'abcde12345')
