/* SSAAS · Frontend shell + reusable components
 * --------------------------------------------------------------
 * Tiny vanilla layer. Provides:
 *   • $/$$/el — DOM helpers
 *   • fmt — formatters
 *   • injectShell + topbar — layout chrome
 *   • Modal — open(title, body, footer)
 *   • Confirm — async yes/no dialog
 *   • Toast — top-right notifications
 *   • ActionMenu — 3-dot row menus
 *   • Form helpers — field(), buildForm(), readForm()
 */

const $  = (sel, root = document) => root.querySelector(sel);
const $$ = (sel, root = document) => Array.from(root.querySelectorAll(sel));

/* ---------- Element factory ---------- */
function el(tag, props = {}, children = []) {
  const node = document.createElement(tag);
  for (const [k, v] of Object.entries(props || {})) {
    if (v == null || v === false) continue;
    if (k === 'class')      node.className = v;
    else if (k === 'style') node.setAttribute('style', v);
    else if (k === 'html')  node.innerHTML = v;
    else if (k.startsWith('on') && typeof v === 'function')
      node.addEventListener(k.slice(2).toLowerCase(), v);
    else if (k === 'data' && typeof v === 'object')
      Object.entries(v).forEach(([dk, dv]) => node.dataset[dk] = dv);
    else node.setAttribute(k, v);
  }
  for (const c of [].concat(children)) {
    if (c == null || c === false) continue;
    node.appendChild(typeof c === 'string' ? document.createTextNode(c) : c);
  }
  return node;
}

/* ---------- Formatters ---------- */
const fmt = {
  pct: (n, d = 1) => (n == null || isNaN(n)) ? '—' : Number(n).toFixed(d) + '%',
  num: (n, d = 1) => (n == null || isNaN(n)) ? '—' : Number(n).toFixed(d),
  int: (n)        => (n == null || isNaN(n)) ? '—' : String(Math.round(Number(n))),
  signed: (n, d = 1) => {
    if (n == null || isNaN(n)) return '—';
    const v = Number(n).toFixed(d);
    return n > 0 ? '+' + v : v;
  },
  date: (iso) => {
    if (!iso) return '—';
    const d = new Date(iso);
    if (isNaN(d)) return iso;
    return d.toLocaleDateString(undefined, { day: '2-digit', month: 'short', year: 'numeric' });
  },
  todayISO: () => new Date().toISOString().slice(0, 10),
};

function severityClass(s) {
  const x = (s || '').toLowerCase();
  if (x === 'critical') return 'severity-critical';
  if (x === 'warning')  return 'severity-warning';
  return 'severity-info';
}

function severityChip(severity) {
  const x = (severity || '').toLowerCase();
  const cls = x === 'critical' ? 'neg' : x === 'warning' ? 'cau' : 'acc';
  return el('span', { class: 'chip ' + cls }, severity);
}

function showError(target, err) {
  console.error(err);
  if (typeof target === 'string') target = $(target);
  if (!target) return;
  target.innerHTML = '';
  target.appendChild(el('div', { class: 'card' }, [
    el('h3', { class: 'card-title text-neg' }, 'Backend unreachable'),
    el('p',  { class: 'card-meta' },
      'Could not reach ssaas_server.exe. Make sure it is running on the configured port.'),
    el('div', { class: 'mono', style: 'font-size: 12px; color: var(--fg-3);' }, err.message),
  ]));
}

function skeleton(width = '60%') {
  return el('span', { class: 'skeleton', style: `width:${width};` });
}

/* ---------- Sidebar / topbar ---------- */
function injectShell() {
  const root = $('.app');
  if (!root) return;
  const main = $('.main', root);

  const navMain = [
    ['nav-overview',    'index.html',       iconHome(),     'Overview'],
    ['nav-subjects',    'subjects.html',    iconBook(),     'Subjects'],
    ['nav-attendance',  'attendance.html',  iconCalendar(), 'Attendance'],
    ['nav-marks',       'marks.html',       iconTrend(),    'Marks'],
    ['nav-exams',       'exams.html',       iconClock(),    'Exams'],
    ['nav-schedule',    'schedule.html',    iconGrid(),     'Schedule'],
  ];
  const navActivities = [
    ['nav-events',       'events.html',       iconStar(),    'Events'],
    ['nav-competitions', 'competitions.html', iconTrophy(),  'Competitions'],
  ];
  const navInsights = [
    ['nav-goals',     'goals.html',     iconTarget(), 'Goals'],
    ['nav-analytics', 'analytics.html', iconLayers(), 'Analytics'],
    ['nav-profile',   'profile.html',   iconUser(),   'Profile'],
  ];

  const sidebar = el('aside', { class: 'sidebar', id: 'sidebar' }, [
    el('div', { class: 'brand' }, [
      el('div', { class: 'brand-mark' }, 'S'),
      el('div', { class: 'brand-text' }, [
        el('div', { class: 'brand-name' }, 'SSAAS'),
        el('div', { class: 'brand-sub'  }, 'student portal'),
      ]),
    ]),
    el('div', { class: 'nav-section' }, 'Academic'),
    el('ul', { class: 'nav' }, navMain.map(navItem)),
    el('div', { class: 'nav-section' }, 'Activities'),
    el('ul', { class: 'nav' }, navActivities.map(navItem)),
    el('div', { class: 'nav-section' }, 'Insights'),
    el('ul', { class: 'nav' }, navInsights.map(navItem)),
    el('div', { class: 'sidebar-footer' }, [
      el('div', {}, 'C++14 backend'),
      el('div', {}, 'Strategy · Observer'),
      el('div', {}, 'Singleton · Factory'),
    ]),
  ]);

  const scrim  = el('div',    { class: 'sidebar-scrim',  id: 'sidebar-scrim',  onClick: closeSidebar });
  const toggle = el('button', { class: 'sidebar-toggle', id: 'sidebar-toggle',
                                'aria-label': 'Open menu', onClick: openSidebar }, iconMenu());

  root.insertBefore(sidebar, main);
  document.body.appendChild(scrim);
  document.body.appendChild(toggle);

  // Modal mounts (one for content, one for confirm).
  document.body.appendChild(el('div', { class: 'modal-backdrop', id: 'modal' }));
  document.body.appendChild(el('div', { class: 'modal-backdrop confirm', id: 'confirm' }));

  // Toast container
  document.body.appendChild(el('div', { id: 'toasts',
    style: 'position: fixed; top: 16px; right: 16px; z-index: 70; display: flex; flex-direction: column; gap: 8px;' }));
}

function navItem([id, href, icon, label]) {
  return el('li', {}, [
    el('a', { id, href }, [
      icon,
      el('span', { class: 'nav-label' }, label),
    ]),
  ]);
}

function openSidebar()  { document.body.classList.add('sidebar-open'); }
function closeSidebar() { document.body.classList.remove('sidebar-open'); }

function setActiveNav(linkId) {
  $$('.nav a').forEach(a => a.classList.remove('active'));
  const a = $('#' + linkId);
  if (a) a.classList.add('active');
}

/* ---------- Topbar ---------- */
function topbar(title, subtitle, student, opts = {}) {
  const initials = (student?.name || 'S')
    .split(' ').map(s => s[0]).filter(Boolean).slice(0, 2).join('').toUpperCase();
  return el('div', { class: 'topbar fade-in' }, [
    el('div', {}, [
      el('h1', {}, title),
      subtitle ? el('div', { class: 'subtitle' }, subtitle) : null,
    ]),
    el('div', { class: 'flex gap-3', style: 'flex-wrap: wrap;' }, [
      opts.showSearch !== false ? el('div', { class: 'search' }, [
        el('span', { class: 'search-icon', html: searchSvg() }),
        el('input', { type: 'text', placeholder: 'Search subjects, events, exams…' }),
      ]) : null,
      el('button', { class: 'bell', 'aria-label': 'Notifications', html: bellSvg() }),
      el('div', { class: 'user-pill' }, [
        el('div', { class: 'user-avatar' }, initials),
        el('div', { class: 'user-meta' }, [
          el('div', { class: 'user-name' }, student?.name || 'Loading…'),
          el('div', { class: 'user-sub'  },
            student ? `${student.rollNumber} · Sem ${student.semester}` : ''),
        ]),
      ]),
    ]),
  ]);
}

/* ---------- Section header ---------- */
function sectionHeader(title, meta = '', actions = []) {
  return el('header', { class: 'section-header' }, [
    el('div', {}, [
      el('h2', { class: 'section-title' }, title),
      meta ? el('div', { class: 'section-meta' }, meta) : null,
    ]),
    actions.length ? el('div', { class: 'section-actions' }, actions) : null,
  ]);
}

/* ---------- Modal ---------- */
const Modal = {
  open({ title, body, footer = null, size = 'standard' } = {}) {
    const root = $('#modal');
    root.innerHTML = '';
    const dialog = el('div', { class: 'modal' + (size === 'wide' ? ' wide' : '') }, [
      el('header', { class: 'modal-head' }, [
        el('h3', { class: 'modal-title' }, title),
        el('button', { class: 'modal-close', 'aria-label': 'Close',
                       onClick: () => Modal.close(), html: closeSvg() }),
      ]),
      el('div', { class: 'modal-body' }, body),
      footer ? el('footer', { class: 'modal-foot' }, footer) : null,
    ]);
    root.appendChild(dialog);
    requestAnimationFrame(() => root.classList.add('open'));
    document.body.style.overflow = 'hidden';
    document.addEventListener('keydown', Modal._esc);
    root.onclick = (e) => { if (e.target === root) Modal.close(); };
  },
  close() {
    const root = $('#modal');
    root.classList.remove('open');
    document.body.style.overflow = '';
    document.removeEventListener('keydown', Modal._esc);
  },
  _esc(e) { if (e.key === 'Escape') Modal.close(); },
};

/* ---------- Confirm ---------- */
function Confirm(message, { confirmLabel = 'Delete', danger = true } = {}) {
  return new Promise((resolve) => {
    const root = $('#confirm');
    root.innerHTML = '';
    const close = (val) => {
      root.classList.remove('open');
      document.body.style.overflow = '';
      resolve(val);
    };
    const dialog = el('div', { class: 'modal' }, [
      el('div', { class: 'modal-body', style: 'padding: 24px;' }, [
        el('div', { class: 'card-title' }, 'Are you sure?'),
        el('p',   { class: 'card-meta',  style: 'margin: 6px 0 0;' }, message),
      ]),
      el('footer', { class: 'modal-foot' }, [
        el('button', { class: 'btn', onClick: () => close(false) }, 'Cancel'),
        el('button', { class: 'btn ' + (danger ? 'btn-danger' : 'btn-primary'),
                       onClick: () => close(true) }, confirmLabel),
      ]),
    ]);
    root.appendChild(dialog);
    requestAnimationFrame(() => root.classList.add('open'));
    document.body.style.overflow = 'hidden';
    root.onclick = (e) => { if (e.target === root) close(false); };
  });
}

/* ---------- Toast ---------- */
function toast(message, kind = 'info') {
  const cont = $('#toasts');
  if (!cont) return;
  const t = el('div', { class: 'toast ' + (kind || '') }, message);
  cont.appendChild(t);
  t.style.animation = 'fadeIn 200ms var(--ease)';
  setTimeout(() => {
    t.style.transition = 'opacity 240ms ease, transform 240ms ease';
    t.style.opacity = '0'; t.style.transform = 'translateY(-4px)';
    setTimeout(() => t.remove(), 280);
  }, 2400);
}

/* ---------- Action menu (3-dot) ---------- */
function actionMenu(items) {
  // items: [{ label, icon?, danger?, onClick }]
  const wrap = el('div', { class: 'menu-wrap' });
  const btn  = el('button', { class: 'btn-icon', 'aria-label': 'Actions', html: dotsSvg() });
  const menu = el('div', { class: 'menu' });
  items.forEach((item, i) => {
    if (item === '---') { menu.appendChild(el('hr')); return; }
    menu.appendChild(el('button', {
      class: item.danger ? 'danger' : '',
      onClick: () => { menu.classList.remove('open'); item.onClick(); },
    }, [
      item.icon ? el('span', { html: item.icon, style: 'width:14px; height:14px; display:inline-block;' }) : null,
      item.label,
    ]));
  });
  btn.addEventListener('click', (e) => {
    e.stopPropagation();
    $$('.menu.open').forEach(m => { if (m !== menu) m.classList.remove('open'); });
    menu.classList.toggle('open');
  });
  document.addEventListener('click', () => menu.classList.remove('open'));
  wrap.appendChild(btn);
  wrap.appendChild(menu);
  return wrap;
}

/* ---------- Form helpers ---------- */
function field(name, label, control) {
  return el('div', { class: 'field' }, [
    el('label', { for: name }, label),
    control,
  ]);
}

function input(name, type = 'text', value = '', extra = {}) {
  return el('input', { name, id: name, type, value, ...extra });
}
function selectField(name, value, options) {
  const sel = el('select', { name, id: name });
  options.forEach(o => {
    const opt = el('option', { value: o.value }, o.label);
    if (o.value === value) opt.setAttribute('selected', 'selected');
    sel.appendChild(opt);
  });
  return sel;
}
function textarea(name, value = '', rows = 3) {
  return el('textarea', { name, id: name, rows }, value);
}

/**
 * Build a <form> element from a schema.
 * schema: [{ name, label, type, value, options?, required?, min?, max?, step?, placeholder?, full? }]
 *   type: text, number, date, time, textarea, select, password, email, checkbox
 *   full: when layout='grid', span both columns
 */
function buildForm(schema, layout = 'stack') {
  const form = el('form', { class: layout === 'grid' ? 'grid-2' : '' });
  schema.forEach(s => {
    let control;
    const extra = {};
    if (s.required)    extra.required = 'required';
    if (s.min != null) extra.min = s.min;
    if (s.max != null) extra.max = s.max;
    if (s.step != null) extra.step = s.step;
    if (s.placeholder) extra.placeholder = s.placeholder;

    if (s.type === 'select') {
      control = selectField(s.name, s.value || '', s.options || []);
    } else if (s.type === 'textarea') {
      control = textarea(s.name, s.value || '');
    } else if (s.type === 'checkbox') {
      control = el('div', { class: 'flex gap-2' }, [
        el('input', { type: 'checkbox', name: s.name, id: s.name,
                      checked: s.value ? 'checked' : null }),
        el('span', { style: 'font-size: 13px; color: var(--fg-2);' }, s.checkboxLabel || ''),
      ]);
    } else {
      control = input(s.name, s.type || 'text', s.value ?? '', extra);
    }
    const isFull = s.full || s.type === 'textarea';
    const wrapper = field(s.name, s.label, control);
    if (isFull && layout === 'grid') wrapper.classList.add('full');
    form.appendChild(wrapper);
  });
  return form;
}

function readForm(form) {
  const data = {};
  Array.from(form.elements).forEach(el => {
    if (!el.name) return;
    if (el.type === 'checkbox') data[el.name] = el.checked;
    else if (el.type === 'number') data[el.name] = el.value === '' ? null : Number(el.value);
    else data[el.name] = el.value;
  });
  return data;
}

/* ---------- Per-code accent (subjects) ---------- */
function codeAccent(code) {
  const palette = ['#4f46e5', '#0ea5e9', '#10b981', '#f59e0b', '#ef4444', '#8b5cf6', '#ec4899', '#14b8a6'];
  let h = 0;
  for (let i = 0; i < (code || '').length; i++) h = (h * 31 + code.charCodeAt(i)) % palette.length;
  return palette[h];
}

/* ---------- Inline icons (sidebar nav) ---------- */
function svg(d, size = 18) {
  return el('svg', {
    class: 'nav-icon', viewBox: '0 0 24 24',
    fill: 'none', stroke: 'currentColor',
    'stroke-width': 1.7, 'stroke-linecap': 'round', 'stroke-linejoin': 'round',
    width: size, height: size,
    html: d,
  });
}
function iconHome()     { return svg('<path d="M3 11l9-8 9 8"/><path d="M5 10v10h14V10"/>'); }
function iconBook()     { return svg('<path d="M4 4h12a4 4 0 0 1 4 4v12H8a4 4 0 0 1-4-4z"/><path d="M4 4v16"/>'); }
function iconCalendar() { return svg('<rect x="3" y="4" width="18" height="18" rx="2"/><path d="M16 2v4M8 2v4M3 10h18"/>'); }
function iconTrend()    { return svg('<path d="M3 17l6-6 4 4 7-7"/><path d="M14 8h6v6"/>'); }
function iconClock()    { return svg('<circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 2"/>'); }
function iconGrid()     { return svg('<rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/>'); }
function iconStar()     { return svg('<path d="M12 2l3 7 7 .5-5.5 4.7L18 22l-6-3.7L6 22l1.5-7.8L2 9.5 9 9z"/>'); }
function iconTrophy()   { return svg('<path d="M8 21h8"/><path d="M12 17v4"/><path d="M7 4h10v5a5 5 0 0 1-10 0z"/><path d="M5 4h2v3a3 3 0 0 1-3-3z"/><path d="M19 4h-2v3a3 3 0 0 0 3-3z"/>'); }
function iconTarget()   { return svg('<circle cx="12" cy="12" r="9"/><circle cx="12" cy="12" r="5"/><circle cx="12" cy="12" r="1.5"/>'); }
function iconLayers()   { return svg('<path d="M12 2l9 5-9 5-9-5z"/><path d="M3 12l9 5 9-5"/><path d="M3 17l9 5 9-5"/>'); }
function iconUser()     { return svg('<circle cx="12" cy="8" r="4"/><path d="M4 21v-1a6 6 0 0 1 6-6h4a6 6 0 0 1 6 6v1"/>'); }
function iconMenu()     { return svg('<path d="M4 6h16M4 12h16M4 18h16"/>', 20); }

function searchSvg() { return '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round" width="16" height="16"><circle cx="11" cy="11" r="7"/><path d="M21 21l-4.3-4.3"/></svg>'; }
function bellSvg()   { return '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round" width="16" height="16"><path d="M18 16v-5a6 6 0 1 0-12 0v5l-2 2h16z"/><path d="M10 21a2 2 0 0 0 4 0"/></svg>'; }
function dotsSvg()   { return '<svg viewBox="0 0 24 24" fill="currentColor" width="16" height="16"><circle cx="12" cy="6" r="1.5"/><circle cx="12" cy="12" r="1.5"/><circle cx="12" cy="18" r="1.5"/></svg>'; }
function closeSvg()  { return '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round" width="16" height="16"><path d="M6 6l12 12M18 6L6 18"/></svg>'; }
function plusSvg()   { return '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round" width="14" height="14"><path d="M12 5v14M5 12h14"/></svg>'; }
function editSvg()   { return '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round" width="14" height="14"><path d="M16 3l5 5L8 21H3v-5z"/></svg>'; }
function trashSvg()  { return '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round" width="14" height="14"><path d="M3 6h18M8 6V4h8v2M6 6l1 14h10l1-14"/></svg>'; }
