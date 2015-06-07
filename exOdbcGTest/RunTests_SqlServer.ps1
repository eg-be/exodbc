#
# RunTests_DB2.ps1
#

param(
	[Parameter(Mandatory=$true)]
	[string]$Target,
	[string]$Dsn="exSqlServer",
	[string]$Uid="ex",
	[string]$Pass="extest",
	[string]$LogLevel="--logLevelW",
	[string]$ConnectionString="Driver={SQL Server Native Client 11.0};Server=192.168.56.20\EXODBC;Database=exodbc;Uid=ex;Pwd=extest;MultipleActiveResultSets=True;"
)

# first run using DSN
Write-Output "Running DSN tests using: " $Target

$filter="*-Excel*"
Write-Output "Filter: " $filter

$ArgsDsn="--gtest_filter=$filter dsn=$Dsn;$Uid;$Pass $LogLevel"
Write-Output "Full Arguments: " $ArgsDsn

$processDsn = Start-Process $Target $ArgsDsn -NoNewWindow -Wait -PassThru
Write-Host "Exit Code: " $processDsn.ExitCode

# and then using connection string
$filter="*-Excel*:*DetectDbms*:*ListDataSources*"
$ArgsCs="--gtest_filter=$filter ""cs=$ConnectionString"" $logLevel"
Write-Output "Full Arguments: " $ArgsCs

$processCs = Start-Process $Target $ArgsCs -NoNewWindow -Wait -PassThru
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
Write-Host "ConnectionString Tests: " $csOk "  Using args: " $ArgsCs

If ($processDsn.ExitCode -eq 0) {
	$dsnOk = "OK"
}
else
{
	$dsnOk = "FAILED"
}
Write-Host "DNS Tests:              " $dsnOk "  Using args: " $ArgsDsn
