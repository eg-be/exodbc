USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[floattypes_tmp]') AND type in (N'U'))
DROP TABLE [exodbc].[floattypes_tmp]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[floattypes_tmp]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[floattypes_tmp](
	[idfloattypes] [int] NOT NULL,
	[tdouble] [float] NULL,
	[tfloat] [float] NULL,
 CONSTRAINT [PK_floattypes_tmp] PRIMARY KEY CLUSTERED 
(
	[idfloattypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
INSERT [exodbc].[floattypes_tmp] ([idfloattypes], [tdouble], [tfloat]) VALUES (1, -3.141592, -3.141)
INSERT [exodbc].[floattypes_tmp] ([idfloattypes], [tdouble], [tfloat]) VALUES (2, 3.141592, 3.141)
