$dict = @{
    "m_windowMode" = "s_isWindowMode"
    "GetWindowMode" = "IsWindowMode"
    
    "m_isCompletedDisplay" = "m_isDisplayingCompletion"
    "IsCompletedDisplay" = "IsDisplayingCompletion"
    
    "m_isRunDone" = "m_hasCompletedRun"
    "m_isJumpDone" = "m_hasCompletedJump"
    "m_isViewDone" = "m_hasCompletedView"
    "m_isMoveDone" = "m_hasCompletedMove"
    
    "m_isMoveCheckAnim" = "m_isPlayingMoveCheckAnim"
    "m_isViewCheckAnim" = "m_isPlayingViewCheckAnim"
    "m_isJumpCheckAnim" = "m_isPlayingJumpCheckAnim"
    "m_isRunCheckAnim" = "m_isPlayingRunCheckAnim"
    
    "s_isLowHealthTutorialShown" = "s_hasShownLowHealthTutorial"
    
    "m_isWave1Loaded" = "m_hasLoadedWave1"
    "m_isWave1EnemySpawned" = "m_hasSpawnedWave1Enemy"
    "m_isShotTutorialCleared" = "m_hasClearedShotTutorial"
    "m_isTackleTutorialCleared" = "m_hasClearedTackleTutorial"
    "m_isRoadFloorBoundsSet" = "m_hasSetRoadFloorBounds"
    "m_isAllWavesCompleted" = "m_haveAllWavesCompleted"
    
    "m_isWave1FirstAidDropped" = "m_hasDroppedWave1FirstAid"
    "m_isWave1AmmoDropped" = "m_hasDroppedWave1Ammo"
    
    "m_ignoreGuardInput" = "m_shouldIgnoreGuardInput"
    "m_isWasRunning" = "m_wasRunning"
    "IsWasRunning" = "WasRunning"
    
    "m_isNoAmmoWarning" = "m_isShowingNoAmmoWarning"
    
    "m_isItemDropped" = "m_hasDroppedItem"
    "m_isTackleHit" = "m_hasTakenTackleDamage"
    "m_isAttackHit" = "m_hasAttackHit"
    
    "s_showDamage" = "s_shouldShowDamage"
    "IsShowDamage" = "ShouldShowDamage"
    
    "s_drawCollision" = "s_shouldDrawCollision"
    "s_drawShieldCollision" = "s_shouldDrawShieldCollision"
    "s_drawAttackHit" = "s_shouldDrawAttackHit"
    
    "s_isDrawCollision" = "s_shouldDrawCollision"
    "IsDrawCollision" = "ShouldDrawCollision"
    
    "s_isDrawTutorialCollision" = "s_shouldDrawTutorialCollision"
    "IsDrawTutorialCollision" = "ShouldDrawTutorialCollision"
    
    "s_isShowActiveEnemyCount" = "s_shouldShowActiveEnemyCount"
    "s_isShowDrawnEnemyCount" = "s_shouldShowDrawnEnemyCount"
    "s_isDrawSpawnAreas" = "s_shouldDrawSpawnAreas"
}

$utf8NoBom = New-Object System.Text.UTF8Encoding $false
Get-ChildItem -Path . -Include *.cpp,*.h -Recurse | ForEach-Object {
    $content = [System.IO.File]::ReadAllText($_.FullName, [System.Text.Encoding]::UTF8)
    $modified = $false
    foreach ($key in $dict.Keys) {
        $val = $dict[$key]
        $pattern = "\b$key\b"
        if ($content -match $pattern) {
            $content = $content -replace $pattern, $val
            $modified = $true
        }
    }
    if ($modified) {
        [System.IO.File]::WriteAllText($_.FullName, $content, $utf8NoBom)
        Write-Output "Updated $($_.Name)"
    }
}
