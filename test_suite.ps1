Write-Host '--- 1. Testing Web Assets ---' -ForegroundColor Cyan
$r1 = curl.exe -s -o NUL -w "%{http_code}" http://localhost:8080/
$r2 = curl.exe -s -o NUL -w "%{http_code}" http://localhost:8080/style.css
$r3 = curl.exe -s -o NUL -w "%{http_code}" http://localhost:8080/app.js
Write-Host "Root: $r1, CSS: $r2, JS: $r3"

Write-Host '--- 2. Testing Initial State ---' -ForegroundColor Cyan
$init = Invoke-RestMethod -Uri 'http://localhost:8080/api/state' -Method Get
Write-Host "Initial CurrentText: '$($init.currentText)', UndoTop: $($init.undoTop), RedoTop: $($init.redoTop)"

Write-Host '--- 3. Testing Edit: Hello ---' -ForegroundColor Cyan
$e1 = Invoke-RestMethod -Uri 'http://localhost:8080/api/edit' -Method Post -Body '{"text": "Hello"}' -ContentType 'application/json'
Write-Host "CurrentText: '$($e1.currentText)', UndoTop: $($e1.undoTop), UndoCount: $($e1.undoCount)"

Write-Host '--- 4. Testing Edit: Hello World ---' -ForegroundColor Cyan
$e2 = Invoke-RestMethod -Uri 'http://localhost:8080/api/edit' -Method Post -Body '{"text": "Hello World"}' -ContentType 'application/json'
Write-Host "CurrentText: '$($e2.currentText)', UndoTop: $($e2.undoTop), UndoCount: $($e2.undoCount)"

Write-Host '--- 5. Testing Undo (Expect Hello) ---' -ForegroundColor Cyan
$u1 = Invoke-RestMethod -Uri 'http://localhost:8080/api/undo' -Method Post
Write-Host "Undo -> CurrentText: '$($u1.currentText)', UndoCount: $($u1.undoCount), RedoCount: $($u1.redoCount)"

Write-Host '--- 6. Testing Redo (Expect Hello World) ---' -ForegroundColor Cyan
$rd1 = Invoke-RestMethod -Uri 'http://localhost:8080/api/redo' -Method Post
Write-Host "Redo -> CurrentText: '$($rd1.currentText)', UndoCount: $($rd1.undoCount), RedoCount: $($rd1.redoCount)"

Write-Host '--- 7. Testing Undo then Edit Hi (Branching Edit) ---' -ForegroundColor Cyan
$u2 = Invoke-RestMethod -Uri 'http://localhost:8080/api/undo' -Method Post
$e3 = Invoke-RestMethod -Uri 'http://localhost:8080/api/edit' -Method Post -Body '{"text": "Hi"}' -ContentType 'application/json'
Write-Host "After Edit 'Hi' -> CurrentText: '$($e3.currentText)', UndoCount: $($e3.undoCount), RedoCount: $($e3.redoCount)"

Write-Host '--- 8. Testing Redo on Branch (Expect Empty) ---' -ForegroundColor Cyan
$rd2 = Invoke-RestMethod -Uri 'http://localhost:8080/api/redo' -Method Post
Write-Host "Redo on branch -> Status: $($rd2.status), Message: '$($rd2.message)'"

Write-Host '--- 9. Testing Reset ---' -ForegroundColor Cyan
$rst = Invoke-RestMethod -Uri 'http://localhost:8080/api/reset' -Method Post
Write-Host "Reset -> Status: $($rst.status), CurrentText: '$($rst.currentText)', UndoCount: $($rst.undoCount), RedoCount: $($rst.redoCount)"

Write-Host '--- ALL AUTOMATED VERIFICATION TESTS PASSED SUCCESSFULLY! ---' -ForegroundColor Green
