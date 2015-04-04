USE [exodbc]
GO

DROP TABLE [exodbc].[blobtypes]
GO

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [exodbc].[blobtypes](
	[idblobtypes] [int] NOT NULL,
	[tblob] [binary](16) NULL,
	[tvarblob_20] [varbinary](20) NULL,
 CONSTRAINT [PK_blobtypes] PRIMARY KEY CLUSTERED 
(
	[idblobtypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
INSERT [exodbc].[blobtypes] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (1, 0x00000000000000000000000000000000, NULL)
INSERT [exodbc].[blobtypes] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (2, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF, NULL)
INSERT [exodbc].[blobtypes] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (3, 0xABCDEFF01234567890ABCDEF01234567, NULL)
INSERT [exodbc].[blobtypes] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (4, NULL, 0xABCDEFF01234567890ABCDEF01234567)
INSERT [exodbc].[blobtypes] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (5, NULL, 0xABCDEFF01234567890ABCDEF01234567FFFFFFFF)
