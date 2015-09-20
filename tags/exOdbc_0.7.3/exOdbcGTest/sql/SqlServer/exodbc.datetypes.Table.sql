USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[datetypes]') AND type in (N'U'))
DROP TABLE [exodbc].[datetypes]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[datetypes]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[datetypes](
	[iddatetypes] [int] NOT NULL,
	[tdate] [date] NULL,
	[ttime] [time](7) NULL,
	[ttimestamp] [datetime] NULL,
 CONSTRAINT [PK_datetypes] PRIMARY KEY CLUSTERED 
(
	[iddatetypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
INSERT [exodbc].[datetypes] ([iddatetypes], [tdate], [ttime], [ttimestamp]) VALUES (1, CAST(N'1983-01-26' AS Date), CAST(N'13:55:56.1234567' AS Time), CAST(N'1983-01-26 13:55:56.000' AS DateTime))
INSERT [exodbc].[datetypes] ([iddatetypes], [tdate], [ttime], [ttimestamp]) VALUES (2, NULL, NULL, CAST(N'1983-01-26 13:55:56.123' AS DateTime))
INSERT [exodbc].[datetypes] ([iddatetypes], [tdate], [ttime], [ttimestamp]) VALUES (3, NULL, NULL, NULL)
INSERT [exodbc].[datetypes] ([iddatetypes], [tdate], [ttime], [ttimestamp]) VALUES (4, NULL, NULL, CAST(N'1983-01-26 13:55:56.010' AS DateTime))
