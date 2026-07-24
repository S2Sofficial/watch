#ifndef WEBPAGE_H
#define WEBPAGE_H

#include <Arduino.h>

// Self-contained single-page interface. Polls /status every 300ms via
// fetch() and posts control actions to the endpoints implemented in
// webserver_manager.cpp. No page reload occurs during normal use.
//
// UI note: purely visual/grayscale minimal styling + a client-side-only
// per-card fullscreen toggle. None of the fetch()/endpoint logic below
// was changed from the functional version.
const char WATCH_WEBPAGE[] PROGMEM = R"HTMLPAGE(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Watch</title>
<style>
  :root{
    --bg:#0d0d0d; --card:#1a1a1a; --border:#333333;
    --text:#eaeaea; --muted:#8a8a8a; --invert-bg:#eaeaea; --invert-text:#0d0d0d;
  }
  *{box-sizing:border-box;}
  body{
    margin:0; font-family:-apple-system,Segoe UI,Roboto,Helvetica,Arial,sans-serif;
    background:var(--bg); color:var(--text); padding:16px;
  }
  body.fs-active{ overflow:hidden; }
  h1{ font-size:1.4rem; text-align:center; margin:8px 0 20px; letter-spacing:2px; font-weight:600; }
  .grid{ display:grid; gap:16px; max-width:520px; margin:0 auto; }
  .card{
    position:relative; background:var(--card); border:1px solid var(--border);
    border-radius:10px; padding:18px 20px;
  }
  .card h2{
    margin:0 0 12px; font-size:0.95rem; text-transform:uppercase;
    letter-spacing:1.5px; color:var(--muted); display:flex; justify-content:space-between; align-items:center;
    padding-right:80px;
  }
  .time{ font-size:2.4rem; font-weight:600; text-align:center; margin:10px 0; letter-spacing:1px; }
  .badge{
    font-size:0.7rem; padding:3px 10px; border-radius:20px; border:1px solid var(--border); color:var(--muted);
  }
  .badge.running{ color:var(--text); border-color:var(--text); }
  .badge.paused{ color:var(--muted); font-style:italic; }
  .badge.finished{ background:var(--invert-bg); color:var(--invert-text); border-color:var(--invert-bg); }
  .row{ display:flex; gap:10px; flex-wrap:wrap; justify-content:center; margin-top:10px; }
  button{
    border:1px solid var(--border); border-radius:8px; padding:10px 16px; font-size:0.9rem; font-weight:600;
    cursor:pointer; color:var(--text); background:#222222; transition:opacity .15s, background .15s;
  }
  button:active{ opacity:0.7; }
  button:hover{ background:#2a2a2a; }
  button.secondary{ background:transparent; }
  button.danger{ background:transparent; color:var(--muted); }
  button.selected{ background:var(--invert-bg); color:var(--invert-text); border-color:var(--invert-bg); }
  .inputs{ display:flex; gap:8px; justify-content:center; margin-bottom:10px; }
  .inputs input{
    width:56px; text-align:center; padding:8px; border-radius:8px; border:1px solid var(--border);
    background:#111111; color:var(--text); font-size:1rem;
  }
  .inputs label{ display:flex; flex-direction:column; align-items:center; font-size:0.7rem; color:var(--muted); gap:4px; }
  .display-select{ display:flex; gap:8px; justify-content:center; }
  .status-dot{
    display:inline-block; width:8px; height:8px; border-radius:50%; margin-right:6px; background:var(--muted);
  }
  .status-dot.ok{ background:var(--text); }

  /* Fullscreen toggle (UI-only; no functional/logic change) */
  .fullscreen-btn{
    position:absolute; top:14px; right:14px; padding:4px 10px; font-size:0.65rem;
    font-weight:500; border-radius:6px; background:transparent; color:var(--muted);
  }
  .fullscreen-btn:hover{ background:#242424; color:var(--text); }
  .grid.has-fullscreen > .card{ display:none; }
  .grid.has-fullscreen > .card.fullscreen{
    display:flex; flex-direction:column; justify-content:center;
    position:fixed; inset:0; width:100vw; height:100vh; z-index:1000;
    border:none; border-radius:0; padding:40px;
  }
  .card.fullscreen .time{ font-size:5rem; margin:24px 0; }
  .card.fullscreen h2{ font-size:1.1rem; padding-right:100px; }
</style>
</head>
<body>

<h1>WATCH</h1>

<div class="grid">

  <div class="card" id="card-selector">
    <button class="fullscreen-btn" onclick="toggleFullscreen('selector', this)">Fullscreen</button>
    <h2>
      <span><span id="connDot" class="status-dot"></span>Display Selector</span>
    </h2>
    <div class="display-select">
      <button id="btnSelClock" onclick="selectDisplay('clock')">Clock</button>
      <button id="btnSelTimer" onclick="selectDisplay('timer')">Timer</button>
      <button id="btnSelStopwatch" onclick="selectDisplay('stopwatch')">Stopwatch</button>
    </div>
  </div>

  <div class="card" id="card-clock">
    <button class="fullscreen-btn" onclick="toggleFullscreen('clock', this)">Fullscreen</button>
    <h2>Clock</h2>
    <div class="time" id="clockTime">--:--:--</div>
  </div>

  <div class="card" id="card-timer">
    <button class="fullscreen-btn" onclick="toggleFullscreen('timer', this)">Fullscreen</button>
    <h2>Timer <span id="timerBadge" class="badge">idle</span></h2>
    <div class="inputs">
      <label>HH<input type="number" id="timerH" min="0" max="99" value="0"></label>
      <label>MM<input type="number" id="timerM" min="0" max="59" value="1"></label>
      <label>SS<input type="number" id="timerS" min="0" max="59" value="0"></label>
    </div>
    <div class="time" id="timerTime">00:00:00</div>
    <div class="row">
      <button onclick="timerAction('start')">Start</button>
      <button class="secondary" onclick="timerAction('pause')">Pause</button>
      <button class="secondary" onclick="timerAction('resume')">Resume</button>
      <button class="danger" onclick="timerAction('reset')">Reset</button>
    </div>
  </div>

  <div class="card" id="card-stopwatch">
    <button class="fullscreen-btn" onclick="toggleFullscreen('stopwatch', this)">Fullscreen</button>
    <h2>Stopwatch <span id="stopwatchBadge" class="badge">idle</span></h2>
    <div class="time" id="stopwatchTime">00:00.00</div>
    <div class="row">
      <button onclick="stopwatchAction('start')">Start</button>
      <button class="secondary" onclick="stopwatchAction('pause')">Pause</button>
      <button class="secondary" onclick="stopwatchAction('resume')">Resume</button>
      <button class="danger" onclick="stopwatchAction('reset')">Reset</button>
    </div>
  </div>

</div>

<script>
let currentDisplay = 'clock';

function badgeClass(prefix, state) {
  if (state === 'running') return 'badge running';
  if (state === 'paused') return 'badge paused';
  if (state === 'finished') return 'badge finished';
  return 'badge';
}

function highlightSelector() {
  document.getElementById('btnSelClock').classList.toggle('selected', currentDisplay === 'clock');
  document.getElementById('btnSelTimer').classList.toggle('selected', currentDisplay === 'timer');
  document.getElementById('btnSelStopwatch').classList.toggle('selected', currentDisplay === 'stopwatch');
}

function refreshStatus() {
  fetch('/status')
    .then(r => r.json())
    .then(data => {
      document.getElementById('connDot').classList.add('ok');

      document.getElementById('clockTime').textContent = data.clock;
      document.getElementById('timerTime').textContent = data.timer;
      document.getElementById('stopwatchTime').textContent = data.stopwatch;

      const timerBadge = document.getElementById('timerBadge');
      timerBadge.textContent = data.timerState;
      timerBadge.className = badgeClass('timer', data.timerState);

      const swBadge = document.getElementById('stopwatchBadge');
      swBadge.textContent = data.stopwatchState;
      swBadge.className = badgeClass('sw', data.stopwatchState);

      currentDisplay = data.display;
      highlightSelector();
    })
    .catch(() => {
      document.getElementById('connDot').classList.remove('ok');
    });
}

function selectDisplay(mode) {
  currentDisplay = mode;
  highlightSelector();
  fetch('/display?mode=' + encodeURIComponent(mode));
}

function timerAction(action) {
  if (action === 'start') {
    const h = document.getElementById('timerH').value || 0;
    const m = document.getElementById('timerM').value || 0;
    const s = document.getElementById('timerS').value || 0;
    const params = new URLSearchParams({ h, m, s });
    fetch('/timer/start', { method: 'POST', headers: {'Content-Type':'application/x-www-form-urlencoded'}, body: params });
  } else {
    fetch('/timer/' + action, { method: 'POST' });
  }
}

function stopwatchAction(action) {
  fetch('/stopwatch/' + action, { method: 'POST' });
}

// ------------------------------------------------------------------
// Fullscreen toggle: UI-only feature. Makes one card fill the
// viewport by toggling CSS classes; does not touch any timer/clock/
// stopwatch/display-selection logic above.
// ------------------------------------------------------------------
function toggleFullscreen(mode, btn) {
  const grid = document.querySelector('.grid');
  const card = document.getElementById('card-' + mode);
  const wasFullscreen = card.classList.contains('fullscreen');

  // Exit whichever card (if any) is currently fullscreen.
  document.querySelectorAll('.card.fullscreen').forEach(c => {
    c.classList.remove('fullscreen');
    const b = c.querySelector('.fullscreen-btn');
    if (b) b.textContent = 'Fullscreen';
  });
  grid.classList.remove('has-fullscreen');
  document.body.classList.remove('fs-active');

  if (!wasFullscreen) {
    card.classList.add('fullscreen');
    grid.classList.add('has-fullscreen');
    document.body.classList.add('fs-active');
    btn.textContent = 'Exit Fullscreen';
  }
}

document.addEventListener('keydown', (e) => {
  if (e.key === 'Escape') {
    const fsCard = document.querySelector('.card.fullscreen');
    if (fsCard) {
      fsCard.classList.remove('fullscreen');
      document.querySelector('.grid').classList.remove('has-fullscreen');
      document.body.classList.remove('fs-active');
      const b = fsCard.querySelector('.fullscreen-btn');
      if (b) b.textContent = 'Fullscreen';
    }
  }
});

setInterval(refreshStatus, 300);
refreshStatus();
</script>

</body>
</html>
)HTMLPAGE";

#endif // WEBPAGE_H
