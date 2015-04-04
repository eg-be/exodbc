USE [exodbc]
GO
DROP TABLE [exodbc].[chartable]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [exodbc].[chartable](
	[idchartable] [int] NOT NULL,
	[col2] [char](128) NULL,
	[col3] [char](128) NULL,
	[col4] [char](128) NULL,
 CONSTRAINT [PK_chartable] PRIMARY KEY CLUSTERED 
(
	[idchartable] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
INSERT [exodbc].[chartable] ([idchartable], [col2], [col3], [col4]) VALUES (1, N'r1_c2                                                                                                                           ', N'r1_c3                                                                                                                           ', N'r1_c4                                                                                                                           ')
