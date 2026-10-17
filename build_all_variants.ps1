<#
.SYNOPSIS
	Configures and builds all litwindow CMake preset variants:
	Windows and Linux, Debug and Release, Shared and Static.

.DESCRIPTION
	Windows variants are built natively using the Visual Studio toolchain
	(located via vswhere) with cmake/ninja from the VsDevCmd environment.

	Linux variants are built inside WSL (Windows Subsystem for Linux), which
	must already have a distribution installed with cmake, ninja, g++ and
	vcpkg available (see CMakePresets.json: linux-base expects vcpkg at
	$HOME/vcpkg inside WSL).

	If WSL is not installed, or no Linux distribution is registered, an
	error is printed and Linux variants are skipped (Windows variants still
	run). This script never touches Visual Studio's own CMake integration,
	so switching between these builds does not trigger a solution reload.

.PARAMETER SkipWindows
	Skip all Windows variants.

.PARAMETER SkipLinux
	Skip all Linux variants (WSL is not queried at all).

.EXAMPLE
	.\build_all_variants.ps1

.EXAMPLE
	.\build_all_variants.ps1 -SkipLinux
#>
[CmdletBinding()]
param(
	[switch]$SkipWindows,
	[switch]$SkipLinux
)

$ErrorActionPreference = 'Stop'

$RepoRoot = $PSScriptRoot
$LogDir = Join-Path $RepoRoot 'out'
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null

$WindowsPresets = @(
	'windows-debug-shared',
	'windows-release-shared',
	'windows-debug-static',
	'windows-release-static'
)

$LinuxPresets = @(
	'linux-debug-shared',
	'linux-release-shared',
	'linux-debug-static',
	'linux-release-static'
)

# Result tracking: preset -> @{ Status = 'ok'|'fail'|'skipped'; Log = path; Message = string }
$Results = [ordered]@{}

function Write-Section($text) {
	Write-Host ''
	Write-Host "==== $text ====" -ForegroundColor Cyan
}

# ---------------------------------------------------------------------------
# Windows build support
# ---------------------------------------------------------------------------

function Find-VsDevCmd {
	$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
	if (-not (Test-Path $vswhere)) {
		return $null
	}
	$installPath = & $vswhere -latest -products * -property installationPath 2>$null
	if (-not $installPath) {
		return $null
	}
	$vsDevCmd = Join-Path $installPath 'Common7\Tools\VsDevCmd.bat'
	if (Test-Path $vsDevCmd) {
		return $vsDevCmd
	}
	return $null
}

function Build-WindowsPreset($preset, $vsDevCmd) {
	$log = Join-Path $LogDir "build-$preset.log"
	Remove-Item -Force -ErrorAction SilentlyContinue $log

	$vsDevCmdCall = '"' + $vsDevCmd + '" -arch=x64 -host_arch=x64 >nul'

	Push-Location $RepoRoot
	try {
		& $env:ComSpec /c "$vsDevCmdCall && cmake --preset $preset" *>> $log
		if ($LASTEXITCODE -ne 0) {
			return @{ Status = 'fail'; Log = $log; Message = "cmake configure failed (exit $LASTEXITCODE)" }
		}

		& $env:ComSpec /c "$vsDevCmdCall && cmake --build out/build/$preset" *>> $log
		if ($LASTEXITCODE -ne 0) {
			return @{ Status = 'fail'; Log = $log; Message = "cmake build failed (exit $LASTEXITCODE)" }
		}
		return @{ Status = 'ok'; Log = $log; Message = 'success' }
	}
	finally {
		Pop-Location
	}
}

# ---------------------------------------------------------------------------
# Linux (WSL) build support
# ---------------------------------------------------------------------------

function Test-WslAvailable {
	$wslCmd = Get-Command wsl.exe -ErrorAction SilentlyContinue
	if (-not $wslCmd) {
		Write-Host "ERROR: WSL (wsl.exe) was not found on this machine. Install WSL (e.g. 'wsl --install') to build the Linux variants." -ForegroundColor Red
		return $false
	}

	# '-l -q' lists installed distributions quietly; non-zero exit or empty
	# output means no distribution is registered.
	$distros = & wsl.exe -l -q 2>$null | Where-Object { $_ -and $_.Trim() -ne '' }
	if ($LASTEXITCODE -ne 0 -or -not $distros) {
		Write-Host "ERROR: WSL is installed, but no Linux distribution is registered. Install one (e.g. 'wsl --install -d Ubuntu') to build the Linux variants." -ForegroundColor Red
		return $false
	}

	return $true
}

function Build-LinuxPreset($preset, $wslRepoRoot) {
	$log = Join-Path $LogDir "build-$preset.log"
	Remove-Item -Force -ErrorAction SilentlyContinue $log

	$script = "cd '$wslRepoRoot' && cmake --preset $preset && cmake --build out/build/$preset"
	& wsl.exe bash -c $script *> $log
	if ($LASTEXITCODE -ne 0) {
		return @{ Status = 'fail'; Log = $log; Message = "configure/build failed (exit $LASTEXITCODE)" }
	}
	return @{ Status = 'ok'; Log = $log; Message = 'success' }
}

# ---------------------------------------------------------------------------
# Windows variants
# ---------------------------------------------------------------------------

if ($SkipWindows) {
	foreach ($p in $WindowsPresets) {
		$Results[$p] = @{ Status = 'skipped'; Log = $null; Message = '-SkipWindows specified' }
	}
}
else {
	Write-Section 'Windows variants'
	$vsDevCmd = Find-VsDevCmd
	if (-not $vsDevCmd) {
		Write-Host 'ERROR: Could not locate VsDevCmd.bat via vswhere. Is Visual Studio with the C++ workload installed?' -ForegroundColor Red
		foreach ($p in $WindowsPresets) {
			$Results[$p] = @{ Status = 'fail'; Log = $null; Message = 'VsDevCmd.bat not found' }
		}
	}
	else {
		foreach ($p in $WindowsPresets) {
			Write-Host "Building $p ..."
			$Results[$p] = Build-WindowsPreset -preset $p -vsDevCmd $vsDevCmd
		}
	}
}

# ---------------------------------------------------------------------------
# Linux variants
# ---------------------------------------------------------------------------

if ($SkipLinux) {
	foreach ($p in $LinuxPresets) {
		$Results[$p] = @{ Status = 'skipped'; Log = $null; Message = '-SkipLinux specified' }
	}
}
else {
	Write-Section 'Linux variants (WSL)'
	if (-not (Test-WslAvailable)) {
		foreach ($p in $LinuxPresets) {
			$Results[$p] = @{ Status = 'skipped'; Log = $null; Message = 'WSL not available' }
		}
	}
	else {
		$wslRepoRoot = (& wsl.exe wslpath -a ($RepoRoot -replace '\\', '/')) 2>$null
		if (-not $wslRepoRoot) {
			$wslRepoRoot = & wsl.exe wslpath -a "$RepoRoot"
		}
		foreach ($p in $LinuxPresets) {
			Write-Host "Building $p ..."
			$Results[$p] = Build-LinuxPreset -preset $p -wslRepoRoot $wslRepoRoot.Trim()
		}
	}
}

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------

Write-Section 'Summary'
$anyFailed = $false
foreach ($p in $Results.Keys) {
	$r = $Results[$p]
	switch ($r.Status) {
		'ok' {
			Write-Host ("  [OK]      {0}" -f $p) -ForegroundColor Green
		}
		'fail' {
			$anyFailed = $true
			Write-Host ("  [FAILED]  {0} - {1} (see {2})" -f $p, $r.Message, $r.Log) -ForegroundColor Red
		}
		'skipped' {
			Write-Host ("  [SKIPPED] {0} - {1}" -f $p, $r.Message) -ForegroundColor Yellow
		}
	}
}

if ($anyFailed) {
	exit 1
}
exit 0
