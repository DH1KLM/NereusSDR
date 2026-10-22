param(
    [string]$QtRoot = 'C:\Qt\6.8.3\msvc2022_64',
    [string]$VSDevCmd = 'C:\Program Files\Microsoft Visual Studio\18\Insiders\Common7\Tools\VsDevCmd.bat',
    [string]$BuildDirectory = 'build-windows',
    [ValidateRange(1, 256)]
    [int]$Jobs = 4,
    [switch]$Reconfigure
)

$ErrorActionPreference = 'Stop'
$ProjectRoot = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$QtCMakeDir = Join-Path $QtRoot 'lib\cmake\Qt6'
if (-not [IO.Path]::IsPathRooted($BuildDirectory)) {
    $BuildDirectory = Join-Path $ProjectRoot $BuildDirectory
}

foreach ($RequiredPath in @($VSDevCmd, "$QtCMakeDir\Qt6Config.cmake", "$ProjectRoot\CMakeLists.txt")) {
    if (-not (Test-Path -LiteralPath $RequiredPath)) {
        throw "Required build dependency not found: $RequiredPath"
    }
}

# //DH1KLM: Import the existing x64 MSVC and Windows SDK environment.
$DeveloperEnvironment = & cmd.exe /d /c "`"$VSDevCmd`" -arch=x64 -host_arch=x64 && set"
if ($LASTEXITCODE -ne 0) { throw 'Visual Studio environment initialization failed.' }
foreach ($Entry in $DeveloperEnvironment) {
    if ($Entry -match '^([^=]+)=(.*)$') {
        [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
    }
}
$env:QT_ROOT_DIR = $QtRoot
$env:Qt6_DIR = $QtCMakeDir
$env:CMAKE_PREFIX_PATH = $QtRoot
$env:Path = "$QtRoot\bin;$env:Path"
# //DH1KLM: RadioModel exceeds MSVC's standard COFF section limit in RelWithDebInfo.
$env:CL = "$env:CL /bigobj"

foreach ($Tool in @('cl.exe', 'cmake.exe', 'ninja.exe')) {
    $null = Get-Command $Tool -ErrorAction Stop
}

# //DH1KLM: Reject relocated caches; configure new builds or an explicit refresh only.
if (Test-Path -LiteralPath "$BuildDirectory\CMakeCache.txt") {
    $CachedRoot = Get-Content "$BuildDirectory\CMakeCache.txt" | Select-String '^CMAKE_HOME_DIRECTORY:INTERNAL=(.*)$'
    if ($CachedRoot -and ([IO.Path]::GetFullPath($CachedRoot.Matches[0].Groups[1].Value) -ne $ProjectRoot)) {
        throw "Build cache belongs to another project path. Select a new -BuildDirectory: $BuildDirectory"
    }
}
if ($Reconfigure -or -not (Test-Path -LiteralPath "$BuildDirectory\build.ninja")) {
    $ConfigureArguments = @('-S', $ProjectRoot, '-B', $BuildDirectory, '-G', 'Ninja',
        '-DCMAKE_BUILD_TYPE=RelWithDebInfo', "-DQt6_DIR=$QtCMakeDir", '-DNEREUS_BUILD_TESTS=OFF',
        '-DFETCHCONTENT_FULLY_DISCONNECTED=ON')
    # //DH1KLM: Reuse downloaded sources without the old build directory's absolute paths.
    foreach ($Source in Get-ChildItem "$ProjectRoot\build\_deps" -Directory -Filter '*-src' -ErrorAction SilentlyContinue) {
        $Dependency = ($Source.Name -replace '-src$', '').ToUpperInvariant()
        $ConfigureArguments += "-DFETCHCONTENT_SOURCE_DIR_${Dependency}=$($Source.FullName)"
    }
    $OpusArchive = "$ProjectRoot\build\third_party\rade\build_opus-prefix\src\940d4e5af64351ca8ba8390df3f555484c567fbb.zip"
    $OpusModel = "$ProjectRoot\build\third_party\rade\opus_model\opus_data-4ed9445b96698bad25d852e912b41495ddfa30c8dbc8a55f9cde5826ed793453.tar.gz"
    foreach ($Archive in @($OpusArchive, $OpusModel)) {
        if (-not (Test-Path -LiteralPath $Archive)) { throw "Required local Opus archive missing: $Archive" }
    }
    # //DH1KLM: CMake verifies both cached Opus archives against its pinned hashes.
    $OpusArchive = $OpusArchive.Replace('\', '/')
    $OpusModel = $OpusModel.Replace('\', '/')
    $ConfigureArguments += @("-DOPUS_URL=$OpusArchive",
        '-DOPUS_URL_HASH=SHA256=20e37f9079ac2b80e3235cd8ce2547829e147bf44a2f5dd28a888fa3e9c24341',
        "-DNEREUS_DEPENDENCY_ARCHIVE_DIR=$([IO.Path]::GetDirectoryName($OpusModel).Replace('\', '/'))")
    & cmake.exe @ConfigureArguments
    if ($LASTEXITCODE -ne 0) { throw 'NereusSDR CMake configuration failed.' }
}

# //DH1KLM: Build application targets and their dependencies without building the test suite.
& cmake.exe --build $BuildDirectory --target NereusSDR nereusd nereus-audio-capture --parallel $Jobs
if ($LASTEXITCODE -ne 0) { throw 'NereusSDR build failed.' }
Write-Host "NereusSDR build succeeded: $BuildDirectory"
