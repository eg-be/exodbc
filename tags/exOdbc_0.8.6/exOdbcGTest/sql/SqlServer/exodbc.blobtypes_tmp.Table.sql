USE [exodbc]
GO
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[blobtypes_tmp]') AND type in (N'U'))
DROP TABLE [exodbc].[blobtypes_tmp]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[exodbc].[blobtypes_tmp]') AND type in (N'U'))
BEGIN
CREATE TABLE [exodbc].[blobtypes_tmp](
	[idblobtypes] [int] NOT NULL,
	[tblob] [binary](16) NULL,
	[tvarblob_20] [varbinary](20) NULL,
 CONSTRAINT [PK_blobtypes_tmp] PRIMARY KEY CLUSTERED 
(
	[idblobtypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]
END
GO
SET ANSI_PADDING OFF
GO
INSERT [exodbc].[blobtypes_tmp] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (100, NULL, 0xABCDEFF01234567890ABCDEF01234567)
INSERT [exodbc].[blobtypes_tmp] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (101, 0x00000000000000000000000000000000, NULL)
