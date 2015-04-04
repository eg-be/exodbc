USE [exodbc]
GO
/****** Object:  Table [exodbc].[floattypes]    Script Date: 04.04.2015 17:12:41 ******/
DROP TABLE [exodbc].[floattypes]
GO
/****** Object:  Table [exodbc].[floattypes]    Script Date: 04.04.2015 17:12:41 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [exodbc].[floattypes](
	[idfloattypes] [int] NOT NULL,
	[tdouble] [float] NULL,
	[tfloat] [float] NULL,
 CONSTRAINT [PK_floattypes] PRIMARY KEY CLUSTERED 
(
	[idfloattypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (1, NULL, 0)
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (2, NULL, 3.141)
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (3, NULL, -3.141)
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (4, 0, NULL)
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (5, 3.141592, NULL)
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (6, -3.141592, NULL)
