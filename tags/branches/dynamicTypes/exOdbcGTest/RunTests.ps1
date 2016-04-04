#
# RunTests.ps1
#

param(
	[Parameter(Mandatory=$true)]
	[string]$Target,
	[string]$Dsn="",
	[string]$Uid="",
	[string]$Pass="",
	[string]$ConnectionString="",
	[string]$LogLevel="--logLevelW",
	[string]$filterDsn="*",
	[string]$filterCs="*",
	[string]$case="l"
)

# first run using DSN
Write-Output "Running tests using: $Target"
Write-Output ""

$ArgsDsn = ""
if($case -eq "l") {
	$ArgsDsn="--gtest_filter=$filterDsn dsn=$Dsn;$Uid;$Pass $LogLevel"
}
else
{
	$ArgsDsn="--gtest_filter=$filterDsn DSN=$Dsn;$Uid;$Pass $LogLevel"
}
Write-Output "DSN-Test: Full Arguments: $ArgsDsn"
Write-Output "========================"
Write-Output ""

$processDsn = Start-Process $Target $ArgsDsn -NoNewWindow -Wait -PassThru
Write-Output ""

# and then using connection string
$ArgsCs = ""
if($case -eq "l") {
	$ArgsCs="--gtest_filter=$filterCs ""cs=$ConnectionString"" $logLevel"
}
else
{
	$ArgsCs="--gtest_filter=$filterCs ""CS=$ConnectionString"" $logLevel"
}
Write-Output "ConnectionString-Test: Full Arguments: $ArgsCs"
Write-Output "======================================"
Write-Output ""

$processCs = Start-Process $Target $ArgsCs -NoNewWindow -Wait -PassThru
Write-Output ""

Write-Output "(DSN) Target: $Target returned with: " $processDsn.ExitCode
Write-Output "(CS)  Target: $Target returned with: " $processCs.ExitCode

# write overall results
Write-Output ""
$dsnExit = $processDsn.ExitCode | Out-String
$dsnOk = ""
If ($processDsn.ExitCode -eq 0) {
	$dsnOk = "OK"
}
else
{
	$dsnOk = "FAILED"
}
Write-Output "DNS Tests:               $dsnOk   Using args:  $ArgsDsn"

$csExit = $processCs.ExitCode | Out-String
$csOk = ""
If ($processCs.ExitCode -eq 0) {
	$csOk = "OK"
}
else
{
	$csOk = "FAILED"
}
Write-Output "ConnectionString Tests:  $csOk   Using args:  $ArgsCs"

if ($processDsn.ExitCode -eq 0 -and $processCs.ExitCode -eq 0)
{
	exit 0;
}
elseif($processDsn.ExitCode -eq 0 -and $processCs.ExitCode -ne 0)
{
	exit 100;
}
elseif($processDsn.ExitCode -ne 0 -and $processCs.ExitCode -eq 0)
{
	exit 10;
}
else
{
	exit 110;
}