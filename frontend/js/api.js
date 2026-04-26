/* SSAAS API client — REST wrapper for the C++ backend.
 * Default base = same origin (when served by ssaas_server.exe).
 */

const API = (() => {
  const sameOrigin = location.protocol === 'http:' || location.protocol === 'https:';
  const base = sameOrigin ? '' : 'http://127.0.0.1:8090';

  async function request(path, opts = {}) {
    const r = await fetch(base + path, opts);
    if (!r.ok) {
      const text = await r.text().catch(() => '');
      let msg = r.statusText;
      try { msg = JSON.parse(text).error || msg; } catch {}
      throw new Error(`API ${r.status}: ${msg}`);
    }
    return r.json();
  }

  function send(method, path, body) {
    return request(path, {
      method,
      headers: { 'Content-Type': 'application/json' },
      body: body == null ? undefined : JSON.stringify(body),
    });
  }

  return {
    base,

    // ---- read ----
    health:        () => request('/api/health'),
    student:       () => request('/api/student'),
    subjects:      () => request('/api/subjects'),
    marks:         () => request('/api/marks'),
    exams:         () => request('/api/exams'),
    events:        () => request('/api/events'),
    competitions:  () => request('/api/competitions'),
    goals:         () => request('/api/goals'),
    alerts:        () => request('/api/alerts'),
    dashboard:     () => request('/api/dashboard'),

    health_score:  () => request('/api/analytics/health'),
    bunk:          () => request('/api/analytics/bunk'),
    risk:          () => request('/api/analytics/risk'),
    performance:   () => request('/api/analytics/performance'),
    pressure:      () => request('/api/analytics/pressure'),
    predict:       () => request('/api/analytics/predict'),
    bayes:         () => request('/api/analytics/bayes'),
    burnout:       () => request('/api/analytics/burnout'),
    examCountdown: () => request('/api/analytics/exams'),
    studyPlan:     () => request('/api/analytics/study-plan'),

    // ---- profile ----
    updateStudent: (patch) => send('PUT', '/api/student', patch),

    // ---- subjects ----
    addSubject:    (s) => send('POST',   '/api/subjects', s),
    updateSubject: (code, s) => send('PUT', `/api/subjects/${encodeURIComponent(code)}`, s),
    deleteSubject: (code)    => send('DELETE', `/api/subjects/${encodeURIComponent(code)}`),

    // ---- marks ----
    addMarks:    (m)        => send('POST',   '/api/marks', m),
    updateMarks: (id, m)    => send('PUT',    `/api/marks/${id}`, m),
    deleteMarks: (id)       => send('DELETE', `/api/marks/${id}`),

    // ---- exams ----
    addExam:    (e)         => send('POST',   '/api/exams', e),
    updateExam: (id, e)     => send('PUT',    `/api/exams/${id}`, e),
    deleteExam: (id)        => send('DELETE', `/api/exams/${id}`),
    setExamPrep:(subject, preparation) => send('POST', '/api/exam-prep', { subject, preparation }),

    // ---- attendance ----
    markAttendance: (subject, present) => send('POST', '/api/attendance', { subject, present }),
    setAttendance:  (code, total, attended) =>
      send('PUT', `/api/attendance/${encodeURIComponent(code)}`, { total, attended }),

    // ---- events ----
    addEvent:    (e)        => send('POST',   '/api/events', e),
    updateEvent: (id, e)    => send('PUT',    `/api/events/${id}`, e),
    deleteEvent: (id)       => send('DELETE', `/api/events/${id}`),

    // ---- competitions ----
    addCompetition:    (c)     => send('POST',   '/api/competitions', c),
    updateCompetition: (id, c) => send('PUT',    `/api/competitions/${id}`, c),
    deleteCompetition: (id)    => send('DELETE', `/api/competitions/${id}`),

    // ---- goals ----
    addGoal:    (g)         => send('POST',   '/api/goals', g),
    updateGoal: (id, g)     => send('PUT',    `/api/goals/${id}`, g),
    deleteGoal: (id)        => send('DELETE', `/api/goals/${id}`),
  };
})();
