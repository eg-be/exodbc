USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[unicodetable_tmp]') AND type in (N'U'))
DROP TABLE [exodbc].[unicodetable_tmp]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[unicodetable_tmp]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[unicodetable_tmp](
	[idunicodetable] [int] NOT NULL,
	[content] [nvarchar](255) NULL,
 CONSTRAINT [PK_unicodetable_tmp] PRIMARY KEY CLUSTERED 
(
	[idunicodetable] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_PADDING OFF
GO
