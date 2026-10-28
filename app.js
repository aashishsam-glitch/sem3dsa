/* ==========================================================================
   UNDO AND REDO USING CHARACTER STACK IMPLEMENTATION
   Pure Vanilla JavaScript Frontend Controller
   1 Stack Element = 1 Character (char items[MAX])
   Top-First LIFO Display & Live Keystroke Synchronization
   ========================================================================== */

const API_BASE = window.location.origin.includes('http') ? window.location.origin : 'http://localhost:8080';
const MAX_CAPACITY = 100;

// Programmatic update guard
let isProgrammaticInput = false;
let autoPlayInterval = null;

// DOM Elements
const currentTextDisplay = document.getElementById('currentTextDisplay');
const currentTextLen = document.getElementById('currentTextLen');
const textInput = document.getElementById('textInput');
const btnUndo = document.getElementById('btnUndo');
const btnUndoWord = document.getElementById('btnUndoWord');
const btnRedo = document.getElementById('btnRedo');
const btnRedoWord = document.getElementById('btnRedoWord');
const btnEdit = document.getElementById('btnEdit');
const btnReset = document.getElementById('btnReset');
const btnDisplay = document.getElementById('btnDisplay');
const undoBadge = document.getElementById('undoBadge');
const redoBadge = document.getElementById('redoBadge');
const undoTopLabel = document.getElementById('undoTopLabel');
const redoTopLabel = document.getElementById('redoTopLabel');
const undoCountLabel = document.getElementById('undoCountLabel');
const redoCountLabel = document.getElementById('redoCountLabel');
const undoStackView = document.getElementById('undoStackView');
const redoStackView = document.getElementById('redoStackView');
const serverStatusBadge = document.getElementById('serverStatusBadge');
const feedbackBanner = document.getElementById('feedbackBanner');
const bannerTitle = document.getElementById('bannerTitle');
const bannerMessage = document.getElementById('bannerMessage');

// --------------------------------------------------------------------------
// 1. API Communication Function (fetch)
// --------------------------------------------------------------------------

async function requestBackend(endpoint, method = 'POST', bodyData = null) {
  const options = {
    method: method,
    headers: {
      'Content-Type': 'application/json'
    }
  };
  if (bodyData !== null) {
    options.body = JSON.stringify(bodyData);
  }

  try {
    const res = await fetch(`${API_BASE}${endpoint}`, options);
    if (!res.ok) {
      throw new Error(`HTTP ${res.status}: ${res.statusText}`);
    }
    const data = await res.json();
    setServerOnline(true);
    return data;
  } catch (err) {
    setServerOnline(false);
    showBanner('danger', 'C Backend Offline', `Unable to reach C server at ${API_BASE}. Ensure server.exe is running.`);
    appendLog('sys', `[ERROR] ${err.message}`);
    throw err;
  }
}

// --------------------------------------------------------------------------
// 2. Character Action Handlers
// --------------------------------------------------------------------------

// 1. Push Single Character to Undo Stack
async function handlePushChar(char) {
  appendLog('c-act', `[REQUEST] POST /api/push_char -> push(&undoStack, '${char === ' ' ? '␣' : char}')`);
  try {
    const state = await requestBackend('/api/push_char', 'POST', { char: char });
    updateUI(state, 'push_char');
  } catch (e) {
    console.error(e);
  }
}

// 2. Apply Full Text (pushes all characters)
async function handleApplyText(customText = null) {
  const text = (customText !== null) ? customText : textInput.value;
  appendLog('c-act', `[REQUEST] POST /api/push_text -> Pushing ${text.length} characters`);
  try {
    const state = await requestBackend('/api/push_text', 'POST', { text: text });
    updateUI(state, 'edit');
  } catch (e) {
    console.error(e);
  }
}

// 3. Undo (1 Character)
async function handleUndo() {
  appendLog('c-act', `[REQUEST] POST /api/undo -> Calling C pop(&undoStack)`);
  try {
    const state = await requestBackend('/api/undo', 'POST');
    // Synchronize textarea with restored text from C
    isProgrammaticInput = true;
    textInput.value = state.currentText || "";
    isProgrammaticInput = false;
    updateUI(state, 'undo');
  } catch (e) {
    console.error(e);
  }
}

// 4. Undo Word
async function handleUndoWord() {
  appendLog('c-act', `[REQUEST] POST /api/undo_word -> Calling C handle_undo_word()`);
  try {
    const state = await requestBackend('/api/undo_word', 'POST');
    isProgrammaticInput = true;
    textInput.value = state.currentText || "";
    isProgrammaticInput = false;
    updateUI(state, 'undo');
  } catch (e) {
    console.error(e);
  }
}

// 5. Redo (1 Character)
async function handleRedo() {
  appendLog('c-act', `[REQUEST] POST /api/redo -> Calling C pop(&redoStack)`);
  try {
    const state = await requestBackend('/api/redo', 'POST');
    isProgrammaticInput = true;
    textInput.value = state.currentText || "";
    isProgrammaticInput = false;
    updateUI(state, 'redo');
  } catch (e) {
    console.error(e);
  }
}

// 6. Redo Word
async function handleRedoWord() {
  appendLog('c-act', `[REQUEST] POST /api/redo_word -> Calling C handle_redo_word()`);
  try {
    const state = await requestBackend('/api/redo_word', 'POST');
    isProgrammaticInput = true;
    textInput.value = state.currentText || "";
    isProgrammaticInput = false;
    updateUI(state, 'redo');
  } catch (e) {
    console.error(e);
  }
}

// 7. Reset All
async function handleReset() {
  if (!confirm('Are you sure you want to reset both character stacks and clear the text?')) {
    return;
  }
  appendLog('c-act', `[REQUEST] POST /api/reset -> Calling C handle_reset()`);
  try {
    const state = await requestBackend('/api/reset', 'POST');
    isProgrammaticInput = true;
    textInput.value = '';
    isProgrammaticInput = false;
    updateUI(state, 'reset');
  } catch (e) {
    console.error(e);
  }
}

// 8. Display Current Text
async function handleDisplay() {
  appendLog('c-act', `[REQUEST] GET /api/display -> Reading currentText from C memory`);
  try {
    const state = await requestBackend('/api/display', 'GET');
    updateUI(state, 'display');
    triggerCardPulse();
  } catch (e) {
    console.error(e);
  }
}

// 9. Initial State Probe
async function fetchInitialState() {
  try {
    const state = await requestBackend('/api/state', 'GET');
    isProgrammaticInput = true;
    textInput.value = state.currentText || "";
    isProgrammaticInput = false;
    updateUI(state, 'init');
    appendLog('ok', `[C BACKEND READY] Character stack capacity: ${state.maxCapacity || MAX_CAPACITY} characters.`);
  } catch (e) {
    console.warn("Backend not yet ready.");
  }
}

// --------------------------------------------------------------------------
// 3. Live Keystroke Synchronization
// --------------------------------------------------------------------------

// When user types in the textarea, send keystrokes directly to the C character stack!
textInput.addEventListener('keydown', async (e) => {
  // Allow system shortcuts
  if (e.ctrlKey || e.metaKey || e.altKey) return;

  // Backspace key -> acts as Undo in the character stack!
  if (e.key === 'Backspace') {
    e.preventDefault();
    if (!btnUndo.disabled) {
      await handleUndo();
    }
    return;
  }

  // Single printable character typed (Letters, numbers, spaces, symbols)
  if (e.key.length === 1) {
    e.preventDefault();
    // Insert character at cursor
    const start = textInput.selectionStart;
    const end = textInput.selectionEnd;
    const val = textInput.value;
    const newVal = val.substring(0, start) + e.key + val.substring(end);

    isProgrammaticInput = true;
    textInput.value = newVal;
    textInput.selectionStart = textInput.selectionEnd = start + 1;
    isProgrammaticInput = false;

    // Send character directly to C undoStack
    await handlePushChar(e.key);
  }
});

// Explicit paste handler to support pasting text from clipboard
textInput.addEventListener('paste', async (e) => {
  e.preventDefault();
  const pastedText = (e.clipboardData || window.clipboardData).getData('text');
  if (!pastedText) return;

  const start = textInput.selectionStart;
  const end = textInput.selectionEnd;
  const val = textInput.value;
  const newVal = val.substring(0, start) + pastedText + val.substring(end);

  isProgrammaticInput = true;
  textInput.value = newVal;
  textInput.selectionStart = textInput.selectionEnd = start + pastedText.length;
  isProgrammaticInput = false;

  await handleApplyText(newVal);
});

// Full text input fallback
textInput.addEventListener('input', (e) => {
  if (isProgrammaticInput) return;
  // If text was pasted or modified externally, sync full string
  handleApplyText(textInput.value);
});

// --------------------------------------------------------------------------
// 4. UI Rendering Engine: Top-First Character Stacks
// --------------------------------------------------------------------------

function updateUI(state, actionType) {
  if (!state) return;

  // 1. Update Current Text Display
  const currentText = state.currentText || "";
  if (currentText.length === 0) {
    currentTextDisplay.innerHTML = `<span class="empty-placeholder">(Empty document text)</span>`;
    currentTextLen.textContent = "0 chars";
  } else {
    currentTextDisplay.textContent = currentText;
    currentTextLen.textContent = `${currentText.length} chars`;
  }

  // 2. Button states & badges
  const undoCount = state.undoCount !== undefined ? state.undoCount : (state.undoStack ? state.undoStack.length : 0);
  const redoCount = state.redoCount !== undefined ? state.redoCount : (state.redoStack ? state.redoStack.length : 0);
  const undoTop = state.undoTop !== undefined ? state.undoTop : (undoCount - 1);
  const redoTop = state.redoTop !== undefined ? state.redoTop : (redoCount - 1);

  btnUndo.disabled = (undoCount === 0);
  btnUndoWord.disabled = (undoCount === 0);
  btnRedo.disabled = (redoCount === 0);
  btnRedoWord.disabled = (redoCount === 0);

  undoBadge.textContent = undoCount;
  redoBadge.textContent = redoCount;

  undoTopLabel.textContent = `top = ${undoTop}`;
  redoTopLabel.textContent = `top = ${redoTop}`;
  undoCountLabel.textContent = `${undoCount} / ${state.maxCapacity || MAX_CAPACITY}`;
  redoCountLabel.textContent = `${redoCount} / ${state.maxCapacity || MAX_CAPACITY}`;

  // 3. Render Top-First Character Stacks
  renderCharacterStack(undoStackView, state.undoStack || [], undoTop, 'undo', actionType);
  renderCharacterStack(redoStackView, state.redoStack || [], redoTop, 'redo', actionType);

  // 4. Status Banner & Logs
  if (state.status === 'success') {
    if (actionType === 'display') {
      showBanner('info', 'Display Current Text', `Active text: "${currentText}" (${currentText.length} chars). Stacks unchanged.`);
      appendLog('ok', `[C RESPONSE] Display -> "${currentText}"`);
    } else if (actionType === 'undo') {
      showBanner('success', 'Undo (Char Popped)', state.message || `Current text: "${currentText}"`);
      appendLog('ok', `[C RESPONSE] Undo -> Active: "${currentText}" | Undo Top: ${undoTop} | Redo Top: ${redoTop}`);
      triggerCardPulse();
    } else if (actionType === 'redo') {
      showBanner('success', 'Redo (Char Restored)', state.message || `Current text: "${currentText}"`);
      appendLog('ok', `[C RESPONSE] Redo -> Active: "${currentText}" | Undo Top: ${undoTop} | Redo Top: ${redoTop}`);
      triggerCardPulse();
    } else if (actionType === 'push_char') {
      showBanner('success', 'Character Pushed', state.message || `Current text: "${currentText}"`);
      appendLog('ok', `[C RESPONSE] Push Char -> Active: "${currentText}"`);
      triggerCardPulse();
    } else if (actionType === 'edit') {
      showBanner('success', 'Text Pushed', state.message || `Current text: "${currentText}"`);
      appendLog('ok', `[C RESPONSE] Push Text -> Active: "${currentText}"`);
      triggerCardPulse();
    } else if (actionType === 'reset') {
      showBanner('info', 'Reset Completed', state.message || `Both character stacks cleared.`);
      appendLog('ok', `[C RESPONSE] Reset -> Stacks cleared (top = -1)`);
    }
  } else if (state.status === 'empty') {
    showBanner('warning', 'Stack Empty Warning', state.message);
    appendLog('warn', `[C RESPONSE] ${state.message}`);
  } else if (state.status === 'error') {
    showBanner('danger', 'Operation Error', state.message);
    appendLog('warn', `[C ERROR] ${state.message}`);
  }
}

// Render each character card with topIndex at the VERY TOP of the cylinder!
function renderCharacterStack(container, items, topIndex, stackType, actionType) {
  container.innerHTML = '';

  // If stack is empty (top == -1)
  if (topIndex < 0 || items.length === 0) {
    const emptyState = document.createElement('div');
    emptyState.className = 'stack-empty-state';
    emptyState.innerHTML = `
      <div class="empty-state-title">${stackType === 'undo' ? 'Undo' : 'Redo'} Stack is Empty (top = -1)</div>
      <div class="empty-state-sub">New characters pushed will appear right here at the <strong>TOP</strong></div>
    `;
    container.appendChild(emptyState);
    return;
  }

  // Loop from topIndex DOWN TO 0 -> Topmost character card is at the very top!
  for (let i = topIndex; i >= 0; i--) {
    const slotDiv = document.createElement('div');
    slotDiv.className = 'stack-slot filled-slot';

    const isTopSlot = (i === topIndex);
    if (isTopSlot) {
      slotDiv.classList.add('is-top');

      // Newly pushed animation
      const isNewPush = (
        (stackType === 'undo' && (actionType === 'push_char' || actionType === 'edit' || actionType === 'redo')) ||
        (stackType === 'redo' && actionType === 'undo')
      );
      if (isNewPush) {
        slotDiv.classList.add('slot-push-anim');
      }

      // Top indicator badge
      const topBadge = document.createElement('span');
      topBadge.className = 'top-pill-badge';
      topBadge.textContent = 'TOP ➔';
      slotDiv.appendChild(topBadge);
    }

    // Slot index [Slot i]
    const indexSpan = document.createElement('span');
    indexSpan.className = 'slot-index';
    indexSpan.textContent = `[${i}]`;
    slotDiv.appendChild(indexSpan);

    // Single Character Chip
    const char = items[i];
    const chip = document.createElement('span');
    chip.className = 'char-chip';

    if (char === ' ') {
      chip.classList.add('special-char');
      chip.textContent = '␣ Space';
    } else if (char === '\n') {
      chip.classList.add('special-char');
      chip.textContent = '↵ Enter';
    } else if (char === '\t') {
      chip.classList.add('special-char');
      chip.textContent = '⇥ Tab';
    } else {
      chip.textContent = `'${char}'`;
    }
    slotDiv.appendChild(chip);

    // Character description label
    const charDesc = document.createElement('span');
    charDesc.className = 'char-desc';
    charDesc.textContent = `Character element #${i}`;
    slotDiv.appendChild(charDesc);

    container.appendChild(slotDiv);
  }
}

// --------------------------------------------------------------------------
// 5. Utility & Helper Functions
// --------------------------------------------------------------------------

function showBanner(type, title, message) {
  feedbackBanner.className = `feedback-banner ${type}`;
  bannerTitle.textContent = title;
  bannerMessage.textContent = message;
  feedbackBanner.style.display = 'flex';
}

function dismissBanner() {
  feedbackBanner.style.display = 'none';
}

function triggerCardPulse() {
  const card = document.querySelector('.current-text-card');
  card.classList.remove('pulse');
  void card.offsetWidth;
  card.classList.add('pulse');
}

function appendLog(type, text) {
  // Console trace log removed from UI
}

function clearConsoleLog() {
  // Console trace log removed from UI
}

function setServerOnline(isOnline) {
  const dot = serverStatusBadge.querySelector('.status-dot');
  const txt = document.getElementById('serverStatusText');
  if (isOnline) {
    dot.className = 'status-dot online';
    txt.textContent = 'C Backend Active';
  } else {
    dot.className = 'status-dot offline';
    txt.textContent = 'C Backend Offline';
  }
}

// --------------------------------------------------------------------------
// 8. Keyboard Shortcuts
// --------------------------------------------------------------------------

window.addEventListener('keydown', (e) => {
  // Ctrl+Z: Undo (1 Character)
  if ((e.ctrlKey || e.metaKey) && !e.shiftKey && (e.key === 'z' || e.key === 'Z')) {
    e.preventDefault();
    if (!btnUndo.disabled) handleUndo();
    return;
  }

  // Ctrl+Shift+Z: Undo Word
  if ((e.ctrlKey || e.metaKey) && e.shiftKey && (e.key === 'z' || e.key === 'Z')) {
    e.preventDefault();
    if (!btnUndoWord.disabled) handleUndoWord();
    return;
  }

  // Ctrl+Y: Redo (1 Character)
  if ((e.ctrlKey || e.metaKey) && (e.key === 'y' || e.key === 'Y')) {
    e.preventDefault();
    if (!btnRedo.disabled) handleRedo();
    return;
  }
});

// Auto-run on load
window.addEventListener('DOMContentLoaded', () => {
  fetchInitialState();
});
