# ===================================================================
# Unreal Engine Clean Script - CleanUEProject.ps1
# Deletes Binaries, Intermediate, DerivedDataCache folders + all *.sln files
# Recursively in the current folder AND all subfolders
# Run this from the root of your Unreal Engine project
# ===================================================================

Write-Host "🚀 Starting Unreal Engine clean..." -ForegroundColor Cyan
Write-Host "This will permanently delete:" -ForegroundColor Yellow
Write-Host "   • All 'Binaries', 'Intermediate', and 'DerivedDataCache' folders" -ForegroundColor Yellow
Write-Host "   • All '*.sln' files (Visual Studio solution files)" -ForegroundColor Yellow
Write-Host "Recursively from the current directory and all subfolders.`n" -ForegroundColor Yellow

$root = Get-Location
Write-Host "Target folder: $root`n" -ForegroundColor Cyan

# Confirm before running (safety first)
$confirm = Read-Host "Are you sure you want to proceed? (type YES to continue)"
if ($confirm -ne "YES") {
    Write-Host "❌ Operation cancelled by user." -ForegroundColor Red
    exit
}

Write-Host "`n🗑️  Deleting folders (Binaries, Intermediate, DerivedDataCache)..." -ForegroundColor Green

# Delete the three folders recursively (including contents)
Get-ChildItem -Path . -Recurse -Directory |
    Where-Object { $_.Name -in @("Binaries", "Intermediate", "DerivedDataCache") } |
    ForEach-Object {
        Write-Host "   Deleting folder: $($_.FullName)" -ForegroundColor DarkGray
        Remove-Item -Path $_.FullName -Recurse -Force -ErrorAction SilentlyContinue
    }

Write-Host "`n🗑️  Deleting all *.sln files..." -ForegroundColor Green

# Delete all .sln files recursively
Get-ChildItem -Path . -Recurse -File -Filter "*.sln" |
    ForEach-Object {
        Write-Host "   Deleting file: $($_.FullName)" -ForegroundColor DarkGray
        Remove-Item -Path $_.FullName -Force -ErrorAction SilentlyContinue
    }

Write-Host "`n✅ Unreal Engine project cleaned successfully!" -ForegroundColor Green
Write-Host "You can now regenerate the project files (right-click .uproject → Generate Visual Studio project files)" -ForegroundColor Cyan
Write-Host "Then rebuild in Visual Studio or use Unreal Editor.`n" -ForegroundColor Cyan