#
# RunTests_DB2.ps1
#

param(
	[Parameter(Mandatory=$true)]
	[string]$Target,
	[string]$Dsn="exDB2",
	[string]$Uid="db2ex",
	[string]$Pass="extest",
	[string]$ConnectionString="Driver={IBM DB2 ODBC DRIVER};Database=EXODBC;Hostname=192.168.56.20;Port=50000;Protocol=TCPIP;Uid=db2ex;Pwd=extest;",
	[string]$LogLevel="--logLevelW",
	[string]$filterDsn="*-*Excel*",
	[string]$filterCs="*-*Excel*:*DetectDbms*:*ListDataSources*",
	[string]$case="u"
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
