USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[selectonly]') AND type in (N'U'))
DROP TABLE [exodbc].[selectonly]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[selectonly]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[selectonly](
	[idselectonly] [int] IDENTITY(1,1) NOT NULL,
	[value] [int] NULL,
 CONSTRAINT [PK_selectonly] PRIMARY KEY CLUSTERED 
(
	[idselectonly] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET IDENTITY_INSERT [exodbc].[selectonly] ON 

INSERT [exodbc].[selectonly] ([idselectonly], [value]) VALUES (1, 1)
INSERT [exodbc].[selectonly] ([idselectonly], [value]) VALUES (2, 1)
INSERT [exodbc].[selectonly] ([idselectonly], [value]) VALUES (3, 1)
SET IDENTITY_INSERT [exodbc].[selectonly] OFF
