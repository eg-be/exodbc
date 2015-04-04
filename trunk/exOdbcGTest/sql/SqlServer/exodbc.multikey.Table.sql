USE [exodbc]
GO
DROP TABLE [exodbc].[multikey]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [exodbc].[multikey](
	[id1] [int] NOT NULL,
	[id2] [int] NOT NULL,
	[value] [nchar](10) NULL,
	[id3] [int] NOT NULL,
 CONSTRAINT [PK_multikey] PRIMARY KEY CLUSTERED 
(
	[id1] ASC,
	[id2] ASC,
	[id3] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
