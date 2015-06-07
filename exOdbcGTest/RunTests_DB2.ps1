#
# RunTests_DB2.ps1
#

param(
	[Parameter(Mandatory=$true)]
	[string]$Target,
	[string]$Dsn="exDB2",
	[string]$Uid="db2ex",
	[string]$Pass="extest",
	[string]$LogLevel="--logLevelW",
	[string]$ConnectionString="Driver={IBM DB2 ODBC DRIVER};Database=EXODBC;Hostname=192.168.56.20;Port=50000;Protocol=TCPIP;Uid=db2ex;Pwd=extest;"
)

# first run using DSN
Write-Output "Running DSN tests using: " $Target

$filter="*-Excel*"
Write-Output "Filter: " $filter

$Args="--gtest_filter=$filter DSN=$Dsn;$Uuid;$Pass $LogLevel"
Write-Output "Full Arguments: " $Args

$processDsn = Start-Process $Target $Args -NoNewWindow -Wait -PassThru
Write-Host "Exit Code: " $processDsn.ExitCode

# and then using connection string
$filter="*-Excel*:*DetectDbms*:*ListDataSources*"
$Args="--gtest_filter=$filter ""CS=$ConnectionString"" $logLevel"
Write-Output "Full Arguments: " $Args

$processCs = Start-Process $Target $Args -NoNewWindow -Wait -PassThru
Write-Host "Exit Code: " $processCs.ExitCode

# write overall results
$dsnOk = ""
$csOk = ""

If ($processCs.ExitCode -eq 0) {
	$csOk = "OK"
}
else
{
	$csOk = "FAILED"
}
Write-Host "ConnectionString Tests: " $csOk

If ($processDsn.ExitCode -eq 0) {
	$dsnOk = "OK"
}
else
{
	$dsnOk = "FAILED"
}
Write-Host "DNS Tests:              " $dsnOk
