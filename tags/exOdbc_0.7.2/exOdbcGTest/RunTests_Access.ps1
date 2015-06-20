#
# RunTests_Access.ps1
#

param(
	[Parameter(Mandatory=$true)]
	[string]$Target,
	[string]$Dsn="exAccess",
	[string]$ConnectionString="Driver={Microsoft Access Driver (*.mdb)};Dbq=E:\exOdbc\exOdbcGTest\access\exodbc.mdb;Uid=Admin;Pwd=;",
	[string]$LogLevel="--logLevelW",
	[string]$filterDsn="*-*Excel*:*TableTest.OpenAutoCheckPrivs*:*TableTest.*UnsupportedColumn*:*TableTest.*Numeric*:*DatabaseTest.ReadSchemas*:*DatabaseTest.ReadTablePrivileges*:*DatabaseTest.ReadTablePrimaryKeysInfo*:*wxCompatibilityTest*",
	[string]$filterCs="*-*Excel*:*DetectDbms*:*ListDataSources*:*TableTest.OpenAutoCheckPrivs*:*TableTest.*UnsupportedColumn*:*TableTest.*Numeric*:*DatabaseTest.ReadSchemas*:*DatabaseTest.ReadTablePrivileges*:*DatabaseTest.ReadTablePrimaryKeysInfo*:*wxCompatibilityTest*",
	[string]$case="l"
)

$ScriptPath = Split-Path $MyInvocation.InvocationName
$args = @()
$args += ("-Target", $Target)
$args += ("-Dsn", $Dsn)
$args += ("-ConnectionString", """$ConnectionString""")
$args += ("-LogLevel", $LogLevel)
$args += ("-filterDsn", $filterDsn)
$args += ("-filterCs", $filterCs)
$args += ("-case", $case)

$cmd = "$ScriptPath\RunTests.ps1"
Write-Host $cmd
Invoke-Expression "$cmd $args"
