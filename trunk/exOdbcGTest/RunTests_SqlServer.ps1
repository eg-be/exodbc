#
# RunTests_SqlServer.ps1
#

param(
	[Parameter(Mandatory=$true)]
	[string]$Target,
	[string]$Dsn="exSqlServer",
	[string]$Uid="ex",
	[string]$Pass="extest",
	[string]$ConnectionString="Driver={SQL Server Native Client 11.0};Server=192.168.56.20\EXODBC;Database=exodbc;Uid=ex;Pwd=extest;MultipleActiveResultSets=True;",
	[string]$LogLevel="--logLevelW",
	[string]$filterDsn="*-Excel*",
	[string]$filterCs="*-Excel*:*DetectDbms*:*ListDataSources*",
	[string]$case="l"
)

$ScriptPath = Split-Path $MyInvocation.InvocationName
$args = @()
$args += ("-Target", $Target)
$args += ("-Dsn", $Dsn)
$args += ("-Uid", $Uid)
$args += ("-Pass", $Pass)
$args += ("-ConnectionString", """$ConnectionString""")
$args += ("-LogLevel", $LogLevel)
$args += ("-filterDsn", $filterDsn)
$args += ("-filterCs", $filterCs)
$args += ("-case", $case)

$cmd = "$ScriptPath\RunTests.ps1"
Write-Host $cmd
Invoke-Expression "$cmd $args"
