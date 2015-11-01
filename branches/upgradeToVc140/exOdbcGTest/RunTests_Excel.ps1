#
# RunTests_Excel.ps1
#

param(
	[Parameter(Mandatory=$true)]
	[string]$Target,
	[string]$Dsn="exExcel",
	[string]$ConnectionString="Driver={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=E:\exOdbc\exOdbcGTest\excel\excelTest.xls;",
	[string]$LogLevel="--logLevelW",
	[string]$filterDsn="*Excel*-*DatabaseTest*:*TableTest*",
	[string]$filterCs="*Excel*-*DatabaseTest*:*TableTest*",
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
