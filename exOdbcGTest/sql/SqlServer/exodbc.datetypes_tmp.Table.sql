USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[datetypes_tmp]') AND type in (N'U'))
DROP TABLE [exodbc].[datetypes_tmp]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[datetypes_tmp]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[datetypes_tmp](
	[iddatetypes] [int] NOT NULL,
	[tdate] [date] NULL,
	[ttime] [time](7) NULL,
	[ttimestamp] [datetime] NULL,
 CONSTRAINT [PK_datetypes_tmp] PRIMARY KEY CLUSTERED 
(
	[iddatetypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
INSERT [exodbc].[datetypes_tmp] ([iddatetypes], [tdate], [ttime], [ttimestamp]) VALUES (1, CAST(N'1983-01-26' AS Date), CAST(N'13:55:56' AS Time), CAST(N'1983-01-26 13:55:56.000' AS DateTime))
