#REQUIRES -Version 2.0

<#  
.SYNOPSIS  
    Update Version information in a VC++ project using subwcrev.
.DESCRIPTION  
	The script will call subwcrev to create a version-header from
	a template-version-header by substituting the $WCREV$, etc 
	entries in the template-file. See subwcrev.
.PARAMETER templateVersionHeader
	Full path to the template header-file. 
.PARAMETER versionHeader
	Full path to the version-header that is generated.
.NOTES  
    File Name      : UpdateVersion.ps1  
    Author         : E. Gerber eg@zame.ch
    Prerequisite   : PowerShell V2 over Vista and upper.
    Copyright 2013 - Elias Gerber  
.LINK  
    Script posted over:  
    http://zame.ch
.EXAMPLE  
    Example 1     
.EXAMPLE    
    Example 2
#>

# Note: Use 'Get-Help ./UpdateVersion.ps1 -detailed' to get detailed help.

Param(
	[Parameter(Mandatory=$True, Position=1)][string]$templateVersionHeader,
	[Parameter(Mandatory=$True, Position=2)][string]$versionHeader
)

# Get the directory of the script
$scriptPath = split-path -parent $MyInvocation.MyCommand.Definition

# Check that template is an existing-file
if (-not(Test-Path -PathType Leaf $templateVersionHeader))
{
	throw "The templateVersionHeader '$templateVersionHeader' does not exist!"
}

# If the header file already exists, remove it
if (Test-Path -PathType Container $versionHeader)
{
	throw "The versionHeader '$versionHeader' is a directory!"
}
if (Test-Path -PathType Leaf $versionHeader)
{
	Remove-Item $versionHeader
}
if (-not(Split-Path -Parent -Path $versionHeader | Test-Path -PathType Container))
{
	throw "The parent-directory of the versionHeader '$versionHeader' does not exist!"
}

# Do subwcrev in the directory of the templateVersionHeader
$templatedir = Split-Path -Parent $templateVersionHeader
$exe = 'subwcrev'
&$exe "$templateDir" "$templateVersionHeader" "$versionHeader"

# Search for the version-info in the now created version-header
$majorRegex = "VERSION_MAJOR\s*(\d+)"
$minorRegex = "VERSION_MINOR\s*(\d+)"
$buildRegex = "VERSION_BUILD\s*(\d+)"
$revRegex = "VERSION_REV\s*(\d+)"
$majorMatch = Select-String -Path $versionHeader -Pattern $majorRegex -AllMatches
$minorMatch = Select-String -Path $versionHeader -Pattern $minorRegex -AllMatches
$buildMatch = Select-String -Path $versionHeader -Pattern $buildRegex -AllMatches
$revMatch = Select-String -Path $versionHeader -Pattern $revRegex -AllMatches
if(-not($majorMatch) -or ($majorMatch.Matches.Count -ne 1))
{
	throw "Not exact one match of regex '$majorRegex' found!"
}
if(-not($minorMatch) -or ($minorMatch.Matches.Count -ne 1))
{
	throw "Not exact one match of regex '$minorRegex' found!"
}
if(-not($buildMatch) -or ($buildMatch.Matches.Count -ne 1))
{
	throw "Not exact one match of regex '$bzRegex' found!"
}
if(-not($revMatch) -or ($revMatch.Matches.Count -ne 1))
{
	throw "Not exact one match of regex '$revRegex' found!"
}
$major = $majorMatch.Matches[0].Groups[1].Value;
$minor = $minorMatch.Matches[0].Groups[1].Value;
$build = $buildMatch.Matches[0].Groups[1].Value;
$rev = $revMatch.Matches[0].Groups[1].Value;

Write-Host "Updated $versionHeader to version major.minor.build.rev: $major.$minor.$build.$rev"

#ForEach-Object {$bla.Matches}

#//$matches[0]
#$matches
#$vMajor = Select-String -Simple "VERSION_MAJOR" "$definitionFile"
#$vMinor = Select-String -Simple "VERSION_MINOR" "$definitionFile"
#$vRevision = Select-String -Simple "VERSION_REV" "$definitionFile"

#if(-not(($vMajor)  -and ($vMinor) -and ($vRevision)))
#{
#	throw "Failed to extract Major, Minor and Revision version-info from $definitionFile"
#}

#Write-Host "Extracted version info: " + $vMajor + "." $vMinor + "." + $vRevision
