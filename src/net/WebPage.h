#pragma once
#include <pgmspace.h>

// Single-page UI: live preview and status, messages, config form and firmware upload
static const char WEB_PAGE[] PROGMEM = R"html(<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Lynx</title>
<style>
:root{--bg:#f5f5f3;--card:#fff;--text:#1c1c1c;--muted:#6b6b6b;--line:#deded9;--accent:#0a6cdb;--error:#c62828;--ok:#2e7d32}
@media (prefers-color-scheme:dark){:root{--bg:#131313;--card:#1d1d1d;--text:#ececec;--muted:#9a9a9a;--line:#333;--accent:#4c9dff;--error:#ff6b6b;--ok:#66bb6a}}
*{box-sizing:border-box}
body{margin:0;background:var(--bg);color:var(--text);font:15px/1.4 system-ui,sans-serif}
main{max-width:640px;margin:0 auto;padding:16px}
h1{font-size:22px;margin:8px 0 16px}
fieldset{border:1px solid var(--line);border-radius:10px;background:var(--card);margin:0 0 16px;padding:12px 16px 4px;min-width:0}
legend{font-weight:600;padding:0 6px}
label{display:block;margin:0 0 12px}
label>span{display:block;font-size:13px;color:var(--muted);margin-bottom:4px}
input,textarea{width:100%;padding:8px;border:1px solid var(--line);border-radius:6px;background:var(--bg);color:var(--text);font:inherit}
input[type=checkbox]{width:auto}
input[type=range]{padding:0;border:0;background:none;accent-color:var(--accent)}
.hint{font-size:12px;color:var(--muted);margin:-8px 0 12px}
table{width:100%;border-collapse:collapse;margin-bottom:8px}
td{padding:4px 2px}
td:nth-child(3) input{width:76px}
td:last-child{white-space:nowrap;text-align:right}
button{font:inherit;padding:8px 14px;border-radius:6px;border:1px solid var(--line);background:var(--card);color:var(--text);cursor:pointer}
button.primary{background:var(--accent);border-color:var(--accent);color:#fff}
button.small{padding:2px 8px}
button:disabled{opacity:.5;cursor:default}
.actions{padding:10px 0;display:flex;gap:12px;align-items:center;flex-wrap:wrap}
.sticky{position:sticky;bottom:0;background:var(--bg)}
.error{color:var(--error)}.ok{color:var(--ok)}
details{margin:0 0 16px}
summary{cursor:pointer;color:var(--muted);margin-bottom:8px}
textarea{height:320px;font:13px ui-monospace,monospace}
progress{width:100%;margin:8px 0}
canvas{display:block;width:100%;aspect-ratio:2/1;image-rendering:pixelated;background:#000;border-radius:6px;margin:4px 0 12px}
dl.info{display:grid;grid-template-columns:auto 1fr;gap:4px 12px;margin:0 0 12px;font-size:14px}
dt{color:var(--muted)}dd{margin:0;overflow-wrap:anywhere}
table.sources{font-size:14px}
th{text-align:left;font-weight:600;color:var(--muted);padding:4px 2px}
td.detail{font-size:12px;color:var(--error);padding-top:0;overflow-wrap:anywhere;text-align:left}
.row{display:flex;gap:12px}.row>label{flex:1}
input[type=color]{height:38px;padding:2px}
</style>
</head>
<body>
<main>
<h1>Lynx</h1>
<fieldset>
<legend>Live</legend>
<canvas id="screen" width="64" height="32"></canvas>
<label><span>Brightness <b id="brightnessValue"></b></span><input id="brightness" type="range" min="5" max="100" step="5" disabled></label>
<div id="info"><p>Loading status...</p></div>
</fieldset>
<fieldset>
<legend>Send a message</legend>
<label><span>Text</span><input id="msgText" maxlength="500"></label>
<div class="row">
<label><span>Duration (s)</span><input id="msgDuration" type="number" min="1" max="600" value="15"></label>
<label><span>Color</span><input id="msgColor" type="color" value="#ffffff"></label>
</div>
<div class="actions"><button id="sendMsg">Send</button><span id="msgStatus"></span></div>
</fieldset>
<p id="loading">Loading settings...</p>
<form id="form" hidden novalidate>
<div id="fields"></div>
<div class="actions sticky"><button class="primary">Save and restart</button><span id="status"></span></div>
</form>
<details id="raw" hidden>
<summary>Edit raw JSON</summary>
<textarea id="json" spellcheck="false"></textarea>
<div class="actions"><button id="saveJson" class="primary">Save JSON and restart</button><span id="jsonStatus"></span></div>
</details>
<fieldset>
<legend>Firmware update</legend>
<label><span>firmware.bin (.pio/build/adafruit_matrixportal_esp32s3/)</span><input id="file" type="file" accept=".bin"></label>
<progress id="progress" value="0" hidden></progress>
<div class="actions"><button id="upload">Install</button><span id="uploadStatus"></span></div>
</fieldset>
</main>
<script>
const MASK = '********';
const SCREENS = {
  clock: 'Clock', weather: 'Current weather', forecast: 'Forecast', hourly: 'Next 12 hours',
  sun: 'Sunrise & sunset', bike: "Velo'v", transport: 'Transport',
};
const SECTIONS = [
  ['Wi-Fi', [
    {key: 'wifi_ssid', label: 'Network name', required: true},
    {key: 'wifi_password', label: 'Password', type: 'secret'},
  ]],
  ['Security', [
    {key: 'admin_password', label: 'Admin password', type: 'secret', hint: 'Protects this page (user "admin") and OTA updates.'},
  ]],
  ['Display', [
    {key: 'brightness', label: 'Initial brightness (%)', type: 'percent', min: 5, max: 100, hint: 'Used until changed with the buttons or the slider above.'},
    {key: 'screen_interval', label: 'Default time per screen (s)', type: 'seconds', min: 1},
    {key: 'tap_threshold', label: 'Tap threshold (0-127)', type: 'integer', min: 0, max: 127, placeholder: '60', hint: 'Tap the case for the next screen, double tap to pause the rotation. Lower is more sensitive, 0 disables it.'},
  ], true],
  ['Location', [
    {key: 'latitude', label: 'Latitude', type: 'number'},
    {key: 'longitude', label: 'Longitude', type: 'number'},
    {key: 'timezone', label: 'Time zone (POSIX TZ)', placeholder: 'CET-1CEST,M3.5.0,M10.5.0/3'},
  ]],
  ['Night mode', [
    {key: 'night_start', label: 'Start', type: 'time'},
    {key: 'night_end', label: 'End', type: 'time'},
    {key: 'night_brightness', label: 'Brightness (%)', type: 'percent', min: 0, max: 100, hint: '0 turns the display off. Leave start and end empty to disable.'},
  ]],
  ["Velo'v", [
    {key: 'api_bike_station_id', label: 'Station ID', type: 'integer'},
  ]],
  ['Transport', [
    {key: 'api_transport_stop_id_1', label: 'Stop ID', type: 'integer'},
    {key: 'api_transport_stop_id_2', label: 'Second stop ID (optional)', type: 'integer'},
    {key: 'api_transport_ligne_cli', label: 'Line', placeholder: 'B'},
    {key: 'api_transport_username', label: 'data.grandlyon username'},
    {key: 'api_transport_password', label: 'data.grandlyon password', type: 'secret'},
  ]],
];
const INPUT_TYPES = {secret: 'password', percent: 'number', seconds: 'number', integer: 'number', number: 'number', time: 'time'};

let config = {};
const $ = id => document.getElementById(id);

function el(tag, props = {}, ...children) {
  const node = Object.assign(document.createElement(tag), props);
  node.append(...children);
  return node;
}

function setStatus(id, text, kind = '') {
  $(id).textContent = text;
  $(id).className = kind;
}

function toInput(field, value) {
  if (value === undefined || value === null) return '';
  switch (field.type) {
    case 'secret': return '';
    case 'percent': return Math.round(value * 100);
    case 'seconds': return value / 1000;
    default: return value;
  }
}

function fromInput(field, input) {
  const raw = input.value.trim();
  // Empty secret = unchanged: the device keeps its value when it receives the mask
  if (field.type === 'secret') return raw || (config[field.key] === MASK ? MASK : undefined);
  if (raw === '') return undefined;
  switch (field.type) {
    case 'percent': return Number(raw) / 100;
    case 'seconds': return Math.round(Number(raw) * 1000);
    case 'integer': return parseInt(raw, 10);
    case 'number': return Number(raw);
    default: return raw;
  }
}

function renderField(field) {
  const input = el('input', {
    id: 'f_' + field.key,
    type: INPUT_TYPES[field.type] || 'text',
    value: toInput(field, config[field.key]),
    required: !!field.required,
    placeholder: field.type === 'secret' ? (config[field.key] === MASK ? 'Unchanged' : 'Not set') : (field.placeholder || ''),
  });
  if (field.type === 'number') input.step = 'any';
  if (field.min !== undefined) input.min = field.min;
  if (field.max !== undefined) input.max = field.max;
  const nodes = [el('label', {}, el('span', {textContent: field.label}), input)];
  if (field.hint) nodes.push(el('p', {className: 'hint', textContent: field.hint}));
  return nodes;
}

// Configured screens first, in their order, then the disabled ones
function renderScreens() {
  const rows = [];
  for (const entry of config.screens || Object.keys(SCREENS)) {
    const row = typeof entry === 'string' ? {name: entry} : {...entry};
    if (SCREENS[row.name] && !rows.some(r => r.name === row.name)) rows.push(row);
  }
  for (const name of Object.keys(SCREENS)) {
    if (!rows.some(r => r.name === name)) rows.push({name, disabled: true});
  }

  const body = el('tbody', {id: 'screens'});
  for (const row of rows) {
    const tr = el('tr');
    tr.dataset.name = row.name;
    const move = delta => {
      const target = delta < 0 ? tr.previousElementSibling : tr.nextElementSibling;
      if (target) body.insertBefore(tr, delta < 0 ? target : target.nextElementSibling);
    };
    tr.append(
      el('td', {}, el('input', {type: 'checkbox', checked: !row.disabled})),
      el('td', {textContent: SCREENS[row.name]}),
      el('td', {}, el('input', {type: 'number', min: 1, step: 'any', placeholder: '-', title: 'Seconds, empty = default', value: row.duration ? row.duration / 1000 : ''})),
      el('td', {},
        el('button', {type: 'button', className: 'small', textContent: '↑', onclick: () => move(-1)}), ' ',
        el('button', {type: 'button', className: 'small', textContent: '↓', onclick: () => move(1)})),
    );
    body.append(tr);
  }
  return [
    el('label', {}, el('span', {textContent: 'Screens (order, duration in s)'})),
    el('table', {}, body),
    el('p', {className: 'hint', textContent: 'Screens whose settings are missing below are skipped.'}),
  ];
}

function render() {
  const container = $('fields');
  container.replaceChildren();
  for (const [title, fields, withScreens] of SECTIONS) {
    const set = el('fieldset', {}, el('legend', {textContent: title}));
    fields.forEach(f => set.append(...renderField(f)));
    if (withScreens) set.append(...renderScreens());
    container.append(set);
  }
}

// Starts from the loaded config so keys unknown to this form are preserved
function buildConfig() {
  const result = {...config};
  for (const [, fields] of SECTIONS) {
    for (const field of fields) {
      const value = fromInput(field, $('f_' + field.key));
      if (value === undefined) delete result[field.key];
      else result[field.key] = value;
    }
  }
  result.screens = [...$('screens').children]
    .filter(tr => tr.querySelector('input[type=checkbox]').checked)
    .map(tr => {
      const seconds = Number(tr.querySelector('input[type=number]').value);
      return seconds > 0 ? {name: tr.dataset.name, duration: Math.round(seconds * 1000)} : tr.dataset.name;
    });
  return result;
}

async function save(body, statusId) {
  setStatus(statusId, 'Saving...');
  try {
    const res = await fetch('/config', {method: 'POST', headers: {'Content-Type': 'application/json'}, body});
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || res.statusText);
    setStatus(statusId, 'Saved. Lynx is restarting...', 'ok');
  } catch (e) {
    setStatus(statusId, e.message, 'error');
  }
}

$('form').addEventListener('submit', e => {
  e.preventDefault();
  if ($('form').reportValidity()) save(JSON.stringify(buildConfig()), 'status');
});

$('raw').addEventListener('toggle', () => {
  if ($('raw').open) $('json').value = JSON.stringify(buildConfig(), null, 2);
});

$('saveJson').addEventListener('click', () => {
  try {
    JSON.parse($('json').value);
  } catch (e) {
    return setStatus('jsonStatus', 'Invalid JSON: ' + e.message, 'error');
  }
  save($('json').value, 'jsonStatus');
});

$('upload').addEventListener('click', () => {
  const file = $('file').files[0];
  if (!file) return setStatus('uploadStatus', 'Choose a firmware file first', 'error');
  const data = new FormData();
  data.append('firmware', file);
  const xhr = new XMLHttpRequest();
  xhr.open('POST', '/update');
  xhr.upload.onprogress = e => { $('progress').value = e.loaded / e.total; };
  xhr.onload = () => {
    $('upload').disabled = false;
    if (xhr.status === 200) setStatus('uploadStatus', 'Installed. Lynx is restarting...', 'ok');
    else setStatus('uploadStatus', xhr.responseText || 'Update failed', 'error');
  };
  xhr.onerror = () => {
    $('upload').disabled = false;
    setStatus('uploadStatus', 'Connection lost', 'error');
  };
  $('upload').disabled = true;
  $('progress').hidden = false;
  setStatus('uploadStatus', 'Uploading, keep this page open...');
  xhr.send(data);
});

function formatDuration(seconds) {
  const d = Math.floor(seconds / 86400), h = Math.floor(seconds % 86400 / 3600), m = Math.floor(seconds % 3600 / 60);
  return d ? `${d}d ${h}h` : h ? `${h}h ${m}min` : m ? `${m}min` : `${seconds}s`;
}

const usedPercent = (free, total) => Math.round(100 * (1 - free / total));

function renderStatus(s) {
  const rows = [
    ['Version', s.version],
    ['Uptime', formatDuration(s.uptime)],
    ['Time', s.time || 'Not synced yet'],
    ['Screen', (SCREENS[s.screen] || s.screen) + (s.paused ? ' (paused)' : '')],
    ['Wi-Fi', s.wifi.connected ? `${s.wifi.ssid} (${s.wifi.rssi} dBm)` : 'Disconnected'],
    ['RAM', `${usedPercent(s.freeHeap, s.heapSize)}% used (peak ${usedPercent(s.minFreeHeap, s.heapSize)}%)`],
    ['Tap sensor', s.tapSensor ? 'On' : 'Off'],
  ];
  syncBrightness(s);
  const info = el('dl', {className: 'info'});
  rows.forEach(([name, value]) => info.append(el('dt', {textContent: name}), el('dd', {textContent: value})));

  const sources = el('table', {className: 'sources'},
    el('tr', {}, ...['Source', 'State', 'Last sync'].map(title => el('th', {textContent: title}))));
  for (const source of s.sources) {
    const state = !source.ok ? 'No data' : source.stale ? 'Outdated' : 'OK';
    sources.append(el('tr', {},
      el('td', {textContent: source.name}),
      el('td', {textContent: state, className: state === 'OK' ? 'ok' : 'error'}),
      el('td', {textContent: source.lastSync === undefined ? '-' : formatDuration(source.lastSync) + ' ago'})));
    if (source.error) sources.append(el('tr', {}, el('td', {colSpan: 3, className: 'detail', textContent: source.error})));
  }
  $('info').replaceChildren(info, sources);
}

// Live brightness: the latest value wins, with at most one request in flight
const brightness = {editedAt: 0, sending: false, queued: null, night: false};

function showBrightness(percent) {
  $('brightnessValue').textContent = percent + '%' + (brightness.night ? ' (night mode)' : '');
}

function syncBrightness(s) {
  brightness.night = s.night;
  // Leaves the slider alone while it is being moved
  if (Date.now() - brightness.editedAt > 6000) $('brightness').value = Math.round(s.brightness * 100);
  $('brightness').disabled = false;
  showBrightness($('brightness').value);
}

async function sendBrightness(percent) {
  if (brightness.sending) {
    brightness.queued = percent;
    return;
  }
  brightness.sending = true;
  try {
    const res = await fetch('/brightness', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({brightness: percent / 100})});
    if (!res.ok) throw new Error(res.statusText);
  } catch (e) {
    brightness.editedAt = 0; // The next status poll shows the actual level
  } finally {
    brightness.sending = false;
    if (brightness.queued !== null) {
      const next = brightness.queued;
      brightness.queued = null;
      sendBrightness(next);
    }
  }
}

$('brightness').addEventListener('input', e => {
  brightness.editedAt = Date.now();
  showBrightness(e.target.value);
  sendBrightness(Number(e.target.value));
});

// RGB565 frame; gain undoes the display dimming so the preview stays readable
function renderScreen(buffer, width, height) {
  const view = new DataView(buffer);
  const pixels = new Uint8ClampedArray(width * height * 4);
  let max = 1;
  for (let i = 0; i < width * height; i++) {
    const c = view.getUint16(i * 2, true);
    pixels.set([(c >> 11) << 3, ((c >> 5) & 63) << 2, (c & 31) << 3, 255], i * 4);
    max = Math.max(max, pixels[i * 4], pixels[i * 4 + 1], pixels[i * 4 + 2]);
  }
  const gain = Math.min(255 / max, 8);
  for (let i = 0; i < pixels.length; i += 4) {
    pixels[i] *= gain;
    pixels[i + 1] *= gain;
    pixels[i + 2] *= gain;
  }
  const canvas = $('screen');
  canvas.width = width;
  canvas.height = height;
  canvas.getContext('2d').putImageData(new ImageData(pixels, width, height), 0, 0);
}

// Sequential polling, so a slow device never gets overlapping requests
async function poll(task, intervalMs) {
  for (;;) {
    if (!document.hidden) {
      try { await task(); } catch (e) { /* Retried on the next round */ }
    }
    await new Promise(resolve => setTimeout(resolve, intervalMs));
  }
}

poll(async () => renderStatus(await (await fetch('/status.json')).json()), 5000);
poll(async () => {
  const res = await fetch('/screen');
  if (res.ok) renderScreen(await res.arrayBuffer(), +res.headers.get('X-Width'), +res.headers.get('X-Height'));
}, 1000);

$('sendMsg').addEventListener('click', async () => {
  const text = $('msgText').value.trim();
  if (!text) return setStatus('msgStatus', 'Type a message first', 'error');
  setStatus('msgStatus', 'Sending...');
  try {
    const body = JSON.stringify({text, duration: Number($('msgDuration').value) || 15, color: $('msgColor').value});
    const res = await fetch('/message', {method: 'POST', headers: {'Content-Type': 'application/json'}, body});
    const data = await res.json();
    if (!res.ok) throw new Error(data.error || res.statusText);
    setStatus('msgStatus', 'Sent', 'ok');
  } catch (e) {
    setStatus('msgStatus', e.message, 'error');
  }
});

fetch('/config.json')
  .then(res => res.ok ? res.json() : Promise.reject(new Error(res.statusText)))
  .then(data => {
    config = data;
    render();
    $('loading').hidden = true;
    $('form').hidden = false;
    $('raw').hidden = false;
  })
  .catch(e => setStatus('loading', 'Cannot load the config: ' + e.message, 'error'));
</script>
</body>
</html>
)html";
