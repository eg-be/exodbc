USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[chartypes]') AND type in (N'U'))
DROP TABLE [exodbc].[chartypes]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[chartypes]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[chartypes](
	[idchartypes] [int] NOT NULL,
	[tvarchar] [nvarchar](128) NULL,
	[tchar] [nchar](128) NULL,
 CONSTRAINT [PK_chartypes] PRIMARY KEY CLUSTERED 
(
	[idchartypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_PADDING OFF
GO
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar]) VALUES (1, N' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~', NULL)
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar]) VALUES (2, NULL, N' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~')
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar]) VALUES (3, N'abcdef', NULL)
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar]) VALUES (4, NULL, N'abcdef')
