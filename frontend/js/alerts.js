/* ============================================================
   SSAAS · Priority-First Alert System
   ------------------------------------------------------------
   • Pure-JS priority engine — scores backend alerts, buckets
     them into P0/P1/P2/P3 tiers.
   • Live notification drawer (bell icon → slide-in).
   • Sticky P0 banner on every page.
   • First-visit-per-day priority briefing modal.
   • Web Audio API synth for P0/P1 chimes (no asset files).
   • Snooze + dismiss with localStorage, per-tier durations.
   • 30 s polling tied to Page Visibility API.
   ============================================================ */

window.Alerts = (() => {

  /* ---------- Priority engine ----------
   *
   *   The C++ backend now computes priorityScore + tier polymorphically
   *   (each IAlert subclass overrides getPriorityScore). This module
   *   prefers the server-supplied values and falls back to a local
   *   reproduction of the formulas if the server is older / offline.
   */

  // Local fallback — mirrors the C++ formulas in the IAlert hierarchy.
  function scoreAlertLocal(a) {
    let s = 0;
    const sev = (a.severity || '').toLowerCase();
    if (sev === 'critical')      s += 60;
    else if (sev === 'warning')  s += 30;
    else                          s += 10;

    const cat = (a.category || '').toLowerCase();

    if (cat === 'exam' && a.daysUntil != null) {
      const d = a.daysUntil;
      if (d <= 1)      s += 30;
      else if (d <= 3) s += 20;
      else if (d <= 7) s += 10;
      if (a.preparation != null && a.preparation < 30) s += 10;
    }
    if (cat === 'attendance' && a.currentPercent != null) {
      s += Math.min(20, Math.max(0, 75 - a.currentPercent));
    }
    if (cat === 'performance' && a.delta != null) {
      s += Math.min(20, Math.abs(a.delta));
    }
    if (cat === 'wellbeing' && a.score != null) {
      s += Math.min(15, Math.max(0, a.score - 30) / 4);
    }
    return Math.min(100, Math.round(s));
  }

  function scoreAlert(a) {
    // Backend-supplied score wins; only fall back if the server is silent.
    return (typeof a.priorityScore === 'number')
      ? Math.round(a.priorityScore)
      : scoreAlertLocal(a);
  }

  function tierFor(score) {
    if (score >= 80) return 'P0';
    if (score >= 60) return 'P1';
    if (score >= 35) return 'P2';
    return 'P3';
  }

  const TIER_META = {
    P0: { label: 'Critical',  color: '#dc2626', icon: '🚨', section: 'Take action now' },
    P1: { label: 'High',      color: '#d97706', icon: '⚠️', section: 'This week' },
    P2: { label: 'Medium',    color: '#ca8a04', icon: '📋', section: 'On your radar' },
    P3: { label: 'Info',      color: '#2563eb', icon: '💡', section: 'Heads up' },
  };

  // Stable id per alert from content (so dismiss/snooze persists across refreshes).
  function alertId(a) {
    return [a.category, a.title, a.subjectCode || '', a.message || '']
      .join('|').replace(/\s+/g, '_');
  }

  function enrich(rawAlerts) {
    return rawAlerts.map(a => {
      const score = scoreAlert(a);
      // Trust server tier if provided; otherwise derive from score.
      const tier  = (typeof a.tier === 'string') ? a.tier : tierFor(score);
      return { ...a, _id: alertId(a), _score: score, _tier: tier };
    }).sort((x, y) => y._score - x._score);
  }

  /* ---------- Snooze / dismiss state ---------- */

  const SNOOZE_HOURS = { P0: 0, P1: 4, P2: 8, P3: 24 };
  const STORE_KEY = 'ssaas.alertState';

  function readState() {
    try { return JSON.parse(localStorage.getItem(STORE_KEY) || '{}'); }
    catch { return {}; }
  }
  function writeState(s) { localStorage.setItem(STORE_KEY, JSON.stringify(s)); }

  function isSnoozed(id) {
    const s = readState();
    const until = s[id]?.snoozedUntil;
    if (!until) return false;
    if (Date.now() > until) {
      delete s[id]; writeState(s);
      return false;
    }
    return true;
  }

  function snooze(alert) {
    const hours = SNOOZE_HOURS[alert._tier];
    if (hours === 0) return false;     // P0 cannot be snoozed
    const s = readState();
    s[alert._id] = { snoozedUntil: Date.now() + hours * 3600_000 };
    writeState(s);
    return true;
  }

  function dismissBannerForToday() {
    const s = readState();
    s.__bannerDismissed = new Date().toISOString().slice(0, 10);
    writeState(s);
  }
  function isBannerDismissedToday() {
    return readState().__bannerDismissed === new Date().toISOString().slice(0, 10);
  }

  function isWelcomeShownToday() {
    return readState().__welcomeShown === new Date().toISOString().slice(0, 10);
  }
  function markWelcomeShown() {
    const s = readState();
    s.__welcomeShown = new Date().toISOString().slice(0, 10);
    writeState(s);
  }

  /* ---------- Sound system (Web Audio API) ----------
   *
   * Synthesises proper bell-like notification dings, not musical tones.
   *
   * A real notification (think iPhone "Tri-tone" or Android "Bell") is
   * a bell timbre: multiple harmonic oscillators stacked together, each
   * with a sharp attack + long exponential decay. We get the metallic
   * "ding" character by detuning the high harmonics slightly so they
   * beat against each other.
   */

  const Sound = {
    ctx: null,
    enabled: localStorage.getItem('ssaas.sound') !== 'off',

    ensureCtx() {
      if (this.ctx) return this.ctx;
      try {
        this.ctx = new (window.AudioContext || window.webkitAudioContext)();
        return this.ctx;
      } catch { return null; }
    },

    // Plays a single bell strike at the given fundamental frequency.
    // Layers fundamental + 2x + ~3x + 4.5x harmonics. Each harmonic gets
    // a quick attack and an exponential decay scaled by its octave —
    // higher partials die faster, mimicking an actual struck bell.
    bell(fundamental, duration = 0.9, volume = 0.18, delay = 0) {
      if (!this.enabled) return;
      const ctx = this.ensureCtx();
      if (!ctx) return;
      if (ctx.state === 'suspended') ctx.resume();
      const t0 = ctx.currentTime + delay;

      // Harmonic stack — slight inharmonic ratios produce the bell shimmer.
      const harmonics = [
        { mult: 1.00,  gain: 1.00, decay: 1.00 }, // fundamental
        { mult: 2.00,  gain: 0.55, decay: 0.85 }, // octave
        { mult: 3.01,  gain: 0.32, decay: 0.65 }, // octave + 5th, detuned
        { mult: 4.50,  gain: 0.16, decay: 0.45 }, // upper partial
        { mult: 5.97,  gain: 0.08, decay: 0.30 }, // shimmer
      ];

      // Master output — soft tube filtering for warmth.
      const master = ctx.createGain();
      master.gain.value = 1.0;
      master.connect(ctx.destination);

      harmonics.forEach(h => {
        const osc  = ctx.createOscillator();
        const gain = ctx.createGain();
        osc.type = 'sine';
        osc.frequency.value = fundamental * h.mult;

        const peak = volume * h.gain;
        const tail = duration * h.decay;
        gain.gain.setValueAtTime(0.0001, t0);
        gain.gain.linearRampToValueAtTime(peak, t0 + 0.004);             // sharp attack
        gain.gain.exponentialRampToValueAtTime(0.0001, t0 + tail);       // long decay

        osc.connect(gain).connect(master);
        osc.start(t0);
        osc.stop(t0 + tail + 0.05);
      });
    },

    // P0 critical alert — two-note urgent ding.
    // Notes cascade quickly (A5 then E5) like phone notification chimes.
    playCritical() {
      this.bell(880.00, 0.95, 0.20);            // A5
      this.bell(659.25, 1.05, 0.16, 0.18);      // E5 — 180 ms after first
    },

    // P1 warning — single softer bell ding.
    playWarning() {
      this.bell(659.25, 0.85, 0.14);            // E5
    },

    // Tick — quick high blip when the drawer opens. Also primes AudioContext.
    playTick() {
      const ctx = this.ensureCtx();
      if (!ctx || !this.enabled) return;
      if (ctx.state === 'suspended') ctx.resume();
      const t0 = ctx.currentTime;
      const osc  = ctx.createOscillator();
      const gain = ctx.createGain();
      osc.type = 'sine';
      osc.frequency.value = 1200;
      gain.gain.setValueAtTime(0.0001, t0);
      gain.gain.linearRampToValueAtTime(0.05, t0 + 0.003);
      gain.gain.exponentialRampToValueAtTime(0.0001, t0 + 0.07);
      osc.connect(gain).connect(ctx.destination);
      osc.start(t0);
      osc.stop(t0 + 0.10);
    },

    // Test sound — same as warning so the user instantly hears the
    // exact alert tone they will get for incoming P1+ notifications.
    playTest() { this.playWarning(); },

    setEnabled(on) {
      this.enabled = !!on;
      localStorage.setItem('ssaas.sound', on ? 'on' : 'off');
      if (on) this.playTest();    // immediate audible confirmation
    },
    // Called once on the first user gesture so the AudioContext is
    // unlocked even before any alert sound needs to play.
    primeOnFirstGesture() {
      if (this._primed) return;
      const handler = () => {
        this._primed = true;
        this.ensureCtx();
        if (this.ctx && this.ctx.state === 'suspended') this.ctx.resume();
        document.removeEventListener('click',    handler, true);
        document.removeEventListener('keydown',  handler, true);
        document.removeEventListener('touchend', handler, true);
      };
      document.addEventListener('click',    handler, true);
      document.addEventListener('keydown',  handler, true);
      document.addEventListener('touchend', handler, true);
    },
  };

  /* ---------- Bell icon binding ---------- */

  function updateBell(activeAlerts) {
    const bell = document.querySelector('.bell');
    if (!bell) return;
    bell.innerHTML = bellSvg();
    bell.classList.remove('has-p0', 'has-p1', 'has-p2', 'has-p3', 'shake');

    // Priority badge
    const counts = { P0: 0, P1: 0, P2: 0, P3: 0 };
    activeAlerts.forEach(a => counts[a._tier]++);

    let topTier = null;
    for (const t of ['P0', 'P1', 'P2', 'P3']) if (counts[t]) { topTier = t; break; }

    if (!topTier) return;
    bell.classList.add('has-' + topTier.toLowerCase());

    const badge = document.createElement('span');
    badge.className = 'bell-badge';
    badge.textContent = counts.P0 + counts.P1 || activeAlerts.length;
    bell.appendChild(badge);
  }

  function shakeBell() {
    const bell = document.querySelector('.bell');
    if (!bell) return;
    bell.classList.remove('shake');
    void bell.offsetWidth;   // restart animation
    bell.classList.add('shake');
  }

  /* ---------- Drawer ---------- */

  function ensureDrawer() {
    let drawer = document.querySelector('#alert-drawer');
    if (drawer) return drawer;
    drawer = document.createElement('aside');
    drawer.id = 'alert-drawer';
    drawer.className = 'alert-drawer';
    drawer.innerHTML = `
      <header class="alert-drawer-head">
        <div>
          <h3>Notifications</h3>
          <div class="alert-drawer-sub" id="alert-drawer-sub">—</div>
        </div>
        <div class="flex gap-2">
          <button class="btn-icon" id="sound-toggle" aria-label="Toggle sound"></button>
          <button class="btn-icon" id="alert-close" aria-label="Close">${closeSvg()}</button>
        </div>
      </header>
      <div class="alert-drawer-body" id="alert-drawer-body"></div>
    `;
    document.body.appendChild(drawer);

    const scrim = document.createElement('div');
    scrim.id = 'alert-scrim';
    scrim.className = 'alert-scrim';
    scrim.addEventListener('click', closeDrawer);
    document.body.appendChild(scrim);

    drawer.querySelector('#alert-close').addEventListener('click', closeDrawer);
    drawer.querySelector('#sound-toggle').addEventListener('click', () => {
      Sound.setEnabled(!Sound.enabled);
      renderSoundButton();
    });
    return drawer;
  }

  function renderSoundButton() {
    const btn = document.querySelector('#sound-toggle');
    if (!btn) return;
    btn.innerHTML = Sound.enabled
      ? '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round" width="16" height="16"><path d="M11 5L6 9H3v6h3l5 4z"/><path d="M15 9a4 4 0 0 1 0 6"/><path d="M18 6a8 8 0 0 1 0 12"/></svg>'
      : '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round" width="16" height="16"><path d="M11 5L6 9H3v6h3l5 4z"/><path d="M22 9l-6 6M16 9l6 6"/></svg>';
    btn.title = Sound.enabled ? 'Sound on' : 'Sound off';
  }

  function openDrawer() {
    ensureDrawer();
    document.body.classList.add('alert-drawer-open');
    renderSoundButton();
    // Bell click is a user gesture — play a tiny tick so the user can confirm
    // sound is working AND so AudioContext gets unlocked for later alerts.
    Sound.playTick();
  }
  function closeDrawer() {
    document.body.classList.remove('alert-drawer-open');
  }

  function renderDrawerContent(activeAlerts) {
    ensureDrawer();
    const body = document.querySelector('#alert-drawer-body');
    const sub  = document.querySelector('#alert-drawer-sub');
    if (!body) return;
    body.innerHTML = '';

    const counts = { P0: 0, P1: 0, P2: 0, P3: 0 };
    activeAlerts.forEach(a => counts[a._tier]++);
    sub.textContent = activeAlerts.length === 0
      ? 'All clear'
      : `${counts.P0} critical · ${counts.P1} high · ${counts.P2 + counts.P3} other`;

    if (activeAlerts.length === 0) {
      const empty = document.createElement('div');
      empty.className = 'alert-empty';
      empty.innerHTML = `
        <div class="alert-empty-icon">✓</div>
        <div class="alert-empty-title">You are all caught up</div>
        <div class="alert-empty-text">No active alerts. We'll let you know if anything needs attention.</div>
      `;
      body.appendChild(empty);
      return;
    }

    ['P0', 'P1', 'P2', 'P3'].forEach(tier => {
      const tierAlerts = activeAlerts.filter(a => a._tier === tier);
      if (!tierAlerts.length) return;
      const meta = TIER_META[tier];
      const section = document.createElement('section');
      section.className = 'alert-tier alert-tier-' + tier.toLowerCase();
      section.innerHTML = `
        <header class="alert-tier-head">
          <span class="alert-tier-icon">${meta.icon}</span>
          <span class="alert-tier-label">${meta.section}</span>
          <span class="alert-tier-count">${tierAlerts.length}</span>
        </header>
      `;
      tierAlerts.forEach(a => section.appendChild(renderAlertCard(a)));
      body.appendChild(section);
    });
  }

  function renderAlertCard(a) {
    const meta = TIER_META[a._tier];
    const card = document.createElement('article');
    card.className = 'alert-card alert-card-' + a._tier.toLowerCase();
    card.dataset.alertId = a._id;

    const pageFor = {
      attendance:  'attendance.html',
      performance: 'marks.html',
      exam:        'exams.html',
      wellbeing:   'analytics.html',
    };
    const target = pageFor[(a.category || '').toLowerCase()] || 'index.html';

    card.innerHTML = `
      <div class="alert-card-head">
        <span class="alert-priority-pill" style="background:${meta.color}20;color:${meta.color};border-color:${meta.color}40;">
          ${a._tier} · ${a._score}
        </span>
        <span class="alert-card-cat">${a.category || ''}</span>
      </div>
      <div class="alert-card-title">${escapeHtml(a.title || '')}</div>
      <div class="alert-card-msg">${escapeHtml(a.message || '')}</div>
      ${a.actionHint ? `<div class="alert-card-hint">→ ${escapeHtml(a.actionHint)}</div>` : ''}
      <div class="alert-card-actions">
        <a class="btn small btn-primary" href="${target}">Take me there</a>
        ${a._tier === 'P0'
          ? '<span class="alert-card-locked">P0 — must be acted upon</span>'
          : `<button class="btn small btn-snooze">Snooze ${SNOOZE_HOURS[a._tier]}h</button>`}
      </div>
    `;

    const snoozeBtn = card.querySelector('.btn-snooze');
    if (snoozeBtn) {
      snoozeBtn.addEventListener('click', () => {
        snooze(a);
        toast(`Snoozed for ${SNOOZE_HOURS[a._tier]} hours`, 'info');
        refresh();
      });
    }
    return card;
  }

  function escapeHtml(s) {
    return String(s).replace(/[&<>"']/g, c =>
      ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  }

  /* ---------- P0 Banner ---------- */

  function ensureBanner() {
    let b = document.querySelector('#alert-banner');
    if (b) return b;
    b = document.createElement('div');
    b.id = 'alert-banner';
    b.className = 'alert-banner';
    document.body.appendChild(b);
    return b;
  }

  function renderBanner(activeAlerts) {
    const banner = ensureBanner();
    const top = activeAlerts.find(a => a._tier === 'P0');
    if (!top || isBannerDismissedToday()) {
      banner.classList.remove('open');
      document.body.classList.remove('has-alert-banner');
      return;
    }
    document.body.classList.add('has-alert-banner');
    const target = ({
      attendance:  'attendance.html',
      performance: 'marks.html',
      exam:        'exams.html',
      wellbeing:   'analytics.html',
    })[(top.category || '').toLowerCase()] || 'index.html';

    banner.innerHTML = `
      <div class="alert-banner-icon">🚨</div>
      <div class="alert-banner-body">
        <div class="alert-banner-title">${escapeHtml(top.title || '')}</div>
        <div class="alert-banner-msg">${escapeHtml(top.message || '')}</div>
      </div>
      <div class="alert-banner-actions">
        <a class="btn btn-primary small" href="${target}">Take me there</a>
        <button class="btn small alert-banner-dismiss">Dismiss for today</button>
      </div>
    `;
    banner.classList.add('open');
    banner.querySelector('.alert-banner-dismiss').addEventListener('click', () => {
      dismissBannerForToday();
      banner.classList.remove('open');
      document.body.classList.remove('has-alert-banner');
    });
  }

  /* ---------- First-visit priority briefing ---------- */

  function maybeShowWelcome(activeAlerts) {
    if (isWelcomeShownToday()) return;
    if (activeAlerts.length === 0) {
      // Friendly empty welcome — first visit per day even if no alerts.
      markWelcomeShown();
      return;
    }
    markWelcomeShown();
    showPriorityBriefing(activeAlerts);
  }

  function showPriorityBriefing(activeAlerts) {
    const top3 = activeAlerts.slice(0, 3);
    const hour = new Date().getHours();
    const greeting = hour < 12 ? 'Good morning'
                   : hour < 17 ? 'Good afternoon'
                   : hour < 21 ? 'Good evening'
                               : 'Late night, eh';

    const list = document.createElement('div');
    list.className = 'briefing-list';
    top3.forEach((a, i) => {
      const meta = TIER_META[a._tier];
      const target = ({
        attendance:  'attendance.html',
        performance: 'marks.html',
        exam:        'exams.html',
        wellbeing:   'analytics.html',
      })[(a.category || '').toLowerCase()] || 'index.html';

      const item = document.createElement('div');
      item.className = 'briefing-item';
      item.innerHTML = `
        <div class="briefing-rank">#${i + 1}</div>
        <div class="briefing-body">
          <div class="briefing-head">
            <span class="alert-priority-pill" style="background:${meta.color}20;color:${meta.color};border-color:${meta.color}40;">
              ${a._tier} · ${a._score}
            </span>
            <span class="briefing-cat">${a.category || ''}</span>
          </div>
          <div class="briefing-title">${escapeHtml(a.title || '')}</div>
          <div class="briefing-msg">${escapeHtml(a.message || '')}</div>
        </div>
        <a class="btn small" href="${target}">Open</a>
      `;
      list.appendChild(item);
    });

    const summary = document.createElement('div');
    summary.className = 'briefing-summary';
    const counts = { P0: 0, P1: 0, P2: 0, P3: 0 };
    activeAlerts.forEach(a => counts[a._tier]++);
    summary.innerHTML = `
      <div class="briefing-greeting">${greeting} 👋</div>
      <div class="briefing-headline">
        ${counts.P0 > 0 ? `<strong style="color:var(--neg)">${counts.P0} critical thing${counts.P0 > 1 ? 's' : ''}</strong>` : ''}
        ${counts.P1 > 0 ? `${counts.P0 ? ', ' : ''}${counts.P1} that need attention this week` : ''}
        ${(counts.P0 || counts.P1) ? '. ' : ''}${(counts.P2 + counts.P3) ? `Plus ${counts.P2 + counts.P3} on your radar.` : ''}
      </div>
    `;

    Modal.open({
      title: '📋 Today\'s priority briefing',
      body: [summary, list],
      footer: [
        el('button', { class: 'btn',         onClick: () => Modal.close() }, 'I got it'),
        el('button', { class: 'btn btn-primary', onClick: () => { Modal.close(); openDrawer(); } }, 'See all alerts'),
      ],
    });

    if (counts.P0 > 0) Sound.playCritical();
    else if (counts.P1 > 0) Sound.playWarning();
  }

  /* ---------- Polling + state ---------- */

  let lastP0Ids = new Set();
  let cached = [];
  let pollTimer = null;

  function activeFromCached() {
    return cached.filter(a => !isSnoozed(a._id));
  }

  async function refresh({ playSoundOnNewP0 = true } = {}) {
    try {
      const r = await fetch((API.base || '') + '/api/alerts');
      if (!r.ok) return;
      const data = await r.json();
      cached = enrich(data.alerts || []);
      const active = activeFromCached();

      // Detect newly arrived P0s
      const currentP0 = new Set(active.filter(a => a._tier === 'P0').map(a => a._id));
      const newP0 = [...currentP0].filter(id => !lastP0Ids.has(id));
      const isFirstLoad = lastP0Ids.size === 0 && cached.length > 0
                          && !window.__alertsBootDone;
      window.__alertsBootDone = true;

      lastP0Ids = currentP0;

      updateBell(active);
      renderDrawerContent(active);
      renderBanner(active);

      if (playSoundOnNewP0 && newP0.length > 0 && !isFirstLoad) {
        Sound.playCritical();
        shakeBell();
        toast('New critical alert', 'neg');
      }

      // First-visit briefing (once per day)
      maybeShowWelcome(active);
    } catch (e) {
      // network error — silent retry next poll
    }
  }

  function startPolling() {
    stopPolling();
    pollTimer = setInterval(() => {
      if (document.hidden) return;        // skip when tab not visible
      refresh();
    }, 30_000);
  }
  function stopPolling() {
    if (pollTimer) { clearInterval(pollTimer); pollTimer = null; }
  }

  function init() {
    // Unlock AudioContext on the first interaction of the session so all
    // subsequent alert chimes play even though browsers block autoplay.
    Sound.primeOnFirstGesture();

    // Wire bell icon (added by injectShell)
    document.addEventListener('click', e => {
      const bell = e.target.closest('.bell');
      if (bell) { e.preventDefault(); openDrawer(); }
    });

    // Page Visibility — pause polling when hidden, refresh on regain focus
    document.addEventListener('visibilitychange', () => {
      if (!document.hidden) refresh();
    });

    // Initial render with empty bell so it has correct base state
    updateBell([]);
    refresh();

    // The bell, search and user-pill are added by topbar() AFTER an async
    // page-load promise resolves, which can race with our first refresh().
    // Watch for the bell to appear and re-render once it does.
    const observer = new MutationObserver(() => {
      if (document.querySelector('.bell') && !window.__alertsRebound) {
        window.__alertsRebound = true;
        observer.disconnect();
        refresh();   // populates bell + banner + briefing now that DOM exists
      }
    });
    observer.observe(document.body, { childList: true, subtree: true });
    // Safety: stop watching after 10 s either way.
    setTimeout(() => observer.disconnect(), 10_000);

    startPolling();
  }

  return { init, refresh, openDrawer, closeDrawer, Sound };
})();
