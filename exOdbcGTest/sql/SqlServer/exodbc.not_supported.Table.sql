USE [exodbc]
GO
/****** Object:  Table [exodbc].[not_supported]    Script Date: 04.04.2015 17:12:41 ******/
DROP TABLE [exodbc].[not_supported]
GO
/****** Object:  Table [exodbc].[not_supported]    Script Date: 04.04.2015 17:12:41 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [exodbc].[not_supported](
	[idnot_supported] [int] NOT NULL,
	[int1] [int] NULL,
	[xml] [xml] NULL,
	[int2] [int] NULL,
 CONSTRAINT [PK_notsupported] PRIMARY KEY CLUSTERED 
(
	[idnot_supported] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]

GO
INSERT [exodbc].[not_supported] ([idnot_supported], [int1], [xml], [int2]) VALUES (1, 10, NULL, 12)
