USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[not_supported_tmp]') AND type in (N'U'))
DROP TABLE [exodbc].[not_supported_tmp]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[not_supported_tmp]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[not_supported_tmp](
	[idnot_supported] [int] NOT NULL,
	[int1] [int] NULL,
	[xml] [xml] NULL,
	[int2] [int] NULL,
 CONSTRAINT [PK_not_supported_tmp] PRIMARY KEY CLUSTERED 
(
	[idnot_supported] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
END
GO
