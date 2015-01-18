USE [master]
GO
/****** Object:  Database [exodbc]    Script Date: 18.01.2015 16:28:11 ******/
CREATE DATABASE [exodbc]
 CONTAINMENT = NONE
 ON  PRIMARY 
( NAME = N'exodbctestdb', FILENAME = N'E:\Microsoft SQL Server\instance\MSSQL12.EXODBC\MSSQL\DATA\exodbctestdb.mdf' , SIZE = 5120KB , MAXSIZE = UNLIMITED, FILEGROWTH = 1024KB )
 LOG ON 
( NAME = N'exodbctestdb_log', FILENAME = N'E:\Microsoft SQL Server\instance\MSSQL12.EXODBC\MSSQL\DATA\exodbctestdb_log.ldf' , SIZE = 2048KB , MAXSIZE = 2048GB , FILEGROWTH = 10%)
GO
ALTER DATABASE [exodbc] SET COMPATIBILITY_LEVEL = 120
GO
IF (1 = FULLTEXTSERVICEPROPERTY('IsFullTextInstalled'))
begin
EXEC [exodbc].[dbo].[sp_fulltext_database] @action = 'enable'
end
GO
ALTER DATABASE [exodbc] SET ANSI_NULL_DEFAULT OFF 
GO
ALTER DATABASE [exodbc] SET ANSI_NULLS OFF 
GO
ALTER DATABASE [exodbc] SET ANSI_PADDING OFF 
GO
ALTER DATABASE [exodbc] SET ANSI_WARNINGS OFF 
GO
ALTER DATABASE [exodbc] SET ARITHABORT OFF 
GO
ALTER DATABASE [exodbc] SET AUTO_CLOSE OFF 
GO
ALTER DATABASE [exodbc] SET AUTO_SHRINK OFF 
GO
ALTER DATABASE [exodbc] SET AUTO_UPDATE_STATISTICS ON 
GO
ALTER DATABASE [exodbc] SET CURSOR_CLOSE_ON_COMMIT OFF 
GO
ALTER DATABASE [exodbc] SET CURSOR_DEFAULT  GLOBAL 
GO
ALTER DATABASE [exodbc] SET CONCAT_NULL_YIELDS_NULL OFF 
GO
ALTER DATABASE [exodbc] SET NUMERIC_ROUNDABORT OFF 
GO
ALTER DATABASE [exodbc] SET QUOTED_IDENTIFIER OFF 
GO
ALTER DATABASE [exodbc] SET RECURSIVE_TRIGGERS OFF 
GO
ALTER DATABASE [exodbc] SET  DISABLE_BROKER 
GO
ALTER DATABASE [exodbc] SET AUTO_UPDATE_STATISTICS_ASYNC OFF 
GO
ALTER DATABASE [exodbc] SET DATE_CORRELATION_OPTIMIZATION OFF 
GO
ALTER DATABASE [exodbc] SET TRUSTWORTHY OFF 
GO
ALTER DATABASE [exodbc] SET ALLOW_SNAPSHOT_ISOLATION ON 
GO
ALTER DATABASE [exodbc] SET PARAMETERIZATION SIMPLE 
GO
ALTER DATABASE [exodbc] SET READ_COMMITTED_SNAPSHOT OFF 
GO
ALTER DATABASE [exodbc] SET HONOR_BROKER_PRIORITY OFF 
GO
ALTER DATABASE [exodbc] SET RECOVERY SIMPLE 
GO
ALTER DATABASE [exodbc] SET  MULTI_USER 
GO
ALTER DATABASE [exodbc] SET PAGE_VERIFY CHECKSUM  
GO
ALTER DATABASE [exodbc] SET DB_CHAINING OFF 
GO
ALTER DATABASE [exodbc] SET FILESTREAM( NON_TRANSACTED_ACCESS = OFF ) 
GO
ALTER DATABASE [exodbc] SET TARGET_RECOVERY_TIME = 0 SECONDS 
GO
ALTER DATABASE [exodbc] SET DELAYED_DURABILITY = DISABLED 
GO
USE [exodbc]
GO
/****** Object:  User [exodbc]    Script Date: 18.01.2015 16:28:11 ******/
CREATE USER [exodbc] FOR LOGIN [exodbc] WITH DEFAULT_SCHEMA=[exodbc]
GO
ALTER ROLE [db_owner] ADD MEMBER [exodbc]
GO
/****** Object:  Schema [exodbc]    Script Date: 18.01.2015 16:28:11 ******/
CREATE SCHEMA [exodbc]
GO
/****** Object:  Table [exodbc].[blobtypes]    Script Date: 18.01.2015 16:28:11 ******/
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
/****** Object:  Table [exodbc].[blobtypes_tmp]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [exodbc].[blobtypes_tmp](
	[idblobtypes] [int] NOT NULL,
	[tblob] [binary](16) NULL,
	[tvarblob_20] [varbinary](20) NULL,
 CONSTRAINT [PK_blobtypes_tmp] PRIMARY KEY CLUSTERED 
(
	[idblobtypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [exodbc].[chartable]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [exodbc].[chartable](
	[idchartable] [int] NOT NULL,
	[col2] [char](128) NULL,
	[col3] [char](128) NULL,
	[col4] [char](128) NULL,
 CONSTRAINT [PK_chartable] PRIMARY KEY CLUSTERED 
(
	[idchartable] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [exodbc].[chartypes]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [exodbc].[chartypes](
	[idchartypes] [int] NOT NULL,
	[tvarchar] [varchar](128) NULL,
	[tchar] [char](128) NULL,
	[tvarchar_10] [nvarchar](10) NULL,
	[tchar_10] [nchar](10) NULL,
 CONSTRAINT [PK_chartypes] PRIMARY KEY CLUSTERED 
(
	[idchartypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [exodbc].[chartypes_tmp]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [exodbc].[chartypes_tmp](
	[idchartypes] [int] NOT NULL,
	[tvarchar] [varchar](128) NULL,
	[tchar] [char](128) NULL,
	[tvarchar_10] [nvarchar](10) NULL,
	[tchar_10] [nchar](10) NULL,
 CONSTRAINT [PK_chartypes_tmp] PRIMARY KEY CLUSTERED 
(
	[idchartypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
SET ANSI_PADDING OFF
GO
/****** Object:  Table [exodbc].[datetypes]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
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

GO
/****** Object:  Table [exodbc].[datetypes_tmp]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
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

GO
/****** Object:  Table [exodbc].[floattypes]    Script Date: 18.01.2015 16:28:11 ******/
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
/****** Object:  Table [exodbc].[floattypes_tmp]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [exodbc].[floattypes_tmp](
	[idfloattypes] [int] NOT NULL,
	[tdouble] [float] NULL,
	[tfloat] [float] NULL,
 CONSTRAINT [PK_floattypes_tmp] PRIMARY KEY CLUSTERED 
(
	[idfloattypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [exodbc].[integertypes]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [exodbc].[integertypes](
	[idintegertypes] [int] NOT NULL,
	[tsmallint] [smallint] NULL,
	[tint] [int] NULL,
	[tbigint] [bigint] NULL,
 CONSTRAINT [PK_integertypes] PRIMARY KEY CLUSTERED 
(
	[idintegertypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [exodbc].[integertypes_tmp]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [exodbc].[integertypes_tmp](
	[idintegertypes] [int] NOT NULL,
	[tsmallint] [smallint] NULL,
	[tint] [int] NULL,
	[tbigint] [bigint] NULL,
 CONSTRAINT [PK_integertypes_tmp] PRIMARY KEY CLUSTERED 
(
	[idintegertypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [exodbc].[multikey]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [exodbc].[multikey](
	[id1] [int] NOT NULL,
	[id2] [int] NOT NULL,
	[value] [nchar](10) NULL,
	[id3] [int] NOT NULL,
 CONSTRAINT [PK_multikey] PRIMARY KEY CLUSTERED 
(
	[id1] ASC,
	[id2] ASC,
	[id3] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [exodbc].[numerictypes]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [exodbc].[numerictypes](
	[idnumerictypes] [int] NOT NULL,
	[tdecimal_18_0] [decimal](18, 0) NULL,
	[tdecimal_18_10] [decimal](18, 10) NULL,
	[tdecimal_5_3] [decimal](5, 3) NULL,
 CONSTRAINT [PK_numerictypes] PRIMARY KEY CLUSTERED 
(
	[idnumerictypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [exodbc].[numerictypes_tmp]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [exodbc].[numerictypes_tmp](
	[idnumerictypes] [int] NOT NULL,
	[tdecimal_18_0] [decimal](18, 0) NULL,
	[tdecimal_18_10] [decimal](18, 10) NULL,
	[tdecimal_5_3] [decimal](5, 3) NULL,
 CONSTRAINT [PK_numerictypes_tmp] PRIMARY KEY CLUSTERED 
(
	[idnumerictypes] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
/****** Object:  Table [exodbc].[selectonly]    Script Date: 18.01.2015 16:28:11 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [exodbc].[selectonly](
	[idselectonly] [int] IDENTITY(1,1) NOT NULL,
	[value] [int] NULL,
 CONSTRAINT [PK_selectonly] PRIMARY KEY CLUSTERED 
(
	[idselectonly] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
) ON [PRIMARY]

GO
INSERT [exodbc].[blobtypes] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (1, 0x00000000000000000000000000000000, NULL)
INSERT [exodbc].[blobtypes] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (2, 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF, NULL)
INSERT [exodbc].[blobtypes] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (3, 0xABCDEFF01234567890ABCDEF01234567, NULL)
INSERT [exodbc].[blobtypes] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (4, NULL, 0xABCDEFF01234567890ABCDEF01234567)
INSERT [exodbc].[blobtypes] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (5, NULL, 0xABCDEFF01234567890ABCDEF01234567FFFFFFFF)
INSERT [exodbc].[blobtypes_tmp] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (100, NULL, 0xABCDEFF01234567890ABCDEF01234567)
INSERT [exodbc].[blobtypes_tmp] ([idblobtypes], [tblob], [tvarblob_20]) VALUES (101, 0x00000000000000000000000000000000, NULL)
INSERT [exodbc].[chartable] ([idchartable], [col2], [col3], [col4]) VALUES (1, N'r1_c2                                                                                                                           ', N'r1_c3                                                                                                                           ', N'r1_c4                                                                                                                           ')
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (1, N' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~', NULL, NULL, NULL)
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (2, NULL, N' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ', NULL, NULL)
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (3, N'äöüàéè', NULL, NULL, NULL)
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (4, NULL, N'äöüàéè                                                                                                                          ', NULL, NULL)
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (5, NULL, NULL, N'abc', N'abc       ')
INSERT [exodbc].[chartypes] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (6, NULL, NULL, N'abcde12345', N'abcde12345')
INSERT [exodbc].[chartypes_tmp] ([idchartypes], [tvarchar], [tchar], [tvarchar_10], [tchar_10]) VALUES (1, N' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~', N' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ', NULL, NULL)
INSERT [exodbc].[datetypes] ([iddatetypes], [tdate], [ttime], [ttimestamp]) VALUES (1, CAST(N'1983-01-26' AS Date), CAST(N'13:55:56.1234567' AS Time), CAST(N'1983-01-26 13:55:56.000' AS DateTime))
INSERT [exodbc].[datetypes] ([iddatetypes], [tdate], [ttime], [ttimestamp]) VALUES (2, NULL, NULL, CAST(N'1983-01-26 13:55:56.123' AS DateTime))
INSERT [exodbc].[datetypes] ([iddatetypes], [tdate], [ttime], [ttimestamp]) VALUES (3, NULL, NULL, NULL)
INSERT [exodbc].[datetypes_tmp] ([iddatetypes], [tdate], [ttime], [ttimestamp]) VALUES (1, CAST(N'1983-01-26' AS Date), CAST(N'13:55:56' AS Time), CAST(N'1983-01-26 13:55:56.000' AS DateTime))
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (1, NULL, 0)
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (2, NULL, 3.141)
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (3, NULL, -3.141)
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (4, 0, NULL)
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (5, 3.141592, NULL)
INSERT [exodbc].[floattypes] ([idfloattypes], [tdouble], [tfloat]) VALUES (6, -3.141592, NULL)
INSERT [exodbc].[floattypes_tmp] ([idfloattypes], [tdouble], [tfloat]) VALUES (1, -3.141592, -3.141)
INSERT [exodbc].[floattypes_tmp] ([idfloattypes], [tdouble], [tfloat]) VALUES (2, 3.141592, 3.141)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (1, -32768, NULL, NULL)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (2, 32767, NULL, NULL)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (3, NULL, -2147483648, NULL)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (4, NULL, 2147483647, NULL)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (5, NULL, NULL, -9223372036854775808)
INSERT [exodbc].[integertypes] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (6, NULL, NULL, 9223372036854775807)
INSERT [exodbc].[integertypes_tmp] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (1, -32768, -2147483648, -9223372036854775808)
INSERT [exodbc].[integertypes_tmp] ([idintegertypes], [tsmallint], [tint], [tbigint]) VALUES (2, 32767, 2147483647, 9223372036854775807)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (1, CAST(0 AS Decimal(18, 0)), NULL, NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (2, CAST(123456789012345678 AS Decimal(18, 0)), NULL, CAST(12.345 AS Decimal(5, 3)))
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (3, CAST(-123456789012345678 AS Decimal(18, 0)), NULL, NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (4, NULL, CAST(0.0000000000 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (5, NULL, CAST(12345678.9012345678 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (6, NULL, CAST(-12345678.9012345678 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes_tmp] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (1, CAST(-123456789012345678 AS Decimal(18, 0)), CAST(-12345678.9012345678 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes_tmp] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (2, CAST(123456789012345678 AS Decimal(18, 0)), CAST(12345678.9012345678 AS Decimal(18, 10)), NULL)
INSERT [exodbc].[numerictypes_tmp] ([idnumerictypes], [tdecimal_18_0], [tdecimal_18_10], [tdecimal_5_3]) VALUES (3, CAST(0 AS Decimal(18, 0)), CAST(0.0000000000 AS Decimal(18, 10)), NULL)
SET IDENTITY_INSERT [exodbc].[selectonly] ON 

INSERT [exodbc].[selectonly] ([idselectonly], [value]) VALUES (1, 1)
INSERT [exodbc].[selectonly] ([idselectonly], [value]) VALUES (2, 1)
INSERT [exodbc].[selectonly] ([idselectonly], [value]) VALUES (3, 1)
SET IDENTITY_INSERT [exodbc].[selectonly] OFF
USE [master]
GO
ALTER DATABASE [exodbc] SET  READ_WRITE 
GO
