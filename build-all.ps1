$env:PICO_SDK_PATH = "C:\Users\khouw\.pico-sdk"

function Build-Pico($boardName) {
    $buildDir = "build-$boardName"

    Write-Host "`n=== Building for $boardName ===" -ForegroundColor Cyan

    if (!(Test-Path $buildDir)) {
        New-Item -ItemType Directory -Path $buildDir | Out-Null
    }

    Push-Location $buildDir

    $env:PICO_BOARD = $boardName

    cmake -G Ninja ..
    ninja

    Pop-Location
}

Build-Pico "pico"
Build-Pico "pico2"