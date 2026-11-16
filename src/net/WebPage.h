#pragma once
#include <pgmspace.h>

// Single-page UI: loads /config.json, renders a form, saves to /config and uploads firmware to /update
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
</style>
</head>
<body>
<main>
<h1>Lynx</h1>
<p id="loading">Loading...</p>
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
const SCREENS = {clock: 'Clock', weather: 'Current weather', forecast: 'Forecast', bike: "Velo'v", transport: 'Transport'};
const SECTIONS = [
  ['Wi-Fi', [
    {key: 'wifi_ssid', label: 'Network name', required: true},
    {key: 'wifi_password', label: 'Password', type: 'secret'},
  ]],
  ['Security', [
    {key: 'admin_password', label: 'Admin password', type: 'secret', hint: 'Protects this page (user "admin") and OTA updates.'},
  ]],
  ['Display', [
    {key: 'brightness', label: 'Initial brightness (%)', type: 'percent', min: 5, max: 100, hint: 'Used until changed with the Up button.'},
    {key: 'screen_interval', label: 'Default time per screen (s)', type: 'seconds', min: 1},
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
