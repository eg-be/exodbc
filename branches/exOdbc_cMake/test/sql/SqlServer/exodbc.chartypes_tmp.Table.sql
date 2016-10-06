USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[chartypes_tmp]') AND type in (N'U'))
DROP TABLE [exodbc].[chartypes_tmp]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[chartypes_tmp]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[chartypes_tmp](
	[idchartypes] [int] NOT NULL,
	[tvarchar] [varchar](128) NULL,
	[tchar] [char](128) NULL,
	[tvarchar_10] [nvarchar](10) NULL,
	[tchar_10] [nchar](10) NULL,
 CONSTRAINT [PK_chartypes_tmp] PRIMARY KEY CLUSTERED 
(
	[idchartypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_PADDING OFF
GO
INSERT [exodbc].[chartypes_tmp] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (1, N' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~', N' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ', NULL, NULL)
