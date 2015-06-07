#
# RunTests_MySql.ps1
#

param(
	[Parameter(Mandatory=$true)]
	[string]$Target,
	[string]$Dsn="exMySql",
	[string]$Uid="ex",
	[string]$Pass="extest",
	[string]$ConnectionString="Provider=MSDASQL;Driver={MySQL ODBC 5.3 UNICODE Driver};Server=192.168.56.20;Database=exodbc;User=ex;Password=extest;Option=3;",
	[string]$LogLevel="--logLevelW",
	[string]$filterDsn="*-Excel*:*TableTest.OpenAutoCheckPrivs*:*TableTest.*UnsupportedColumn*:*TableTest.OpenAutoCheckPrivs*",
	[string]$filterCs="*-Excel*:*DetectDbms*:*ListDataSources*:*TableTest.OpenAutoCheckPrivs*:*TableTest.*UnsupportedColumn*:*TableTest.OpenAutoCheckPrivs*",
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
