/* SSAAS · Chart.js helpers — light-theme palette
 * Single primary (indigo) plus a desaturated paired accent
 * for multi-series. Semantic colors only when carrying data.
 */

const CHART = {
  fg:      '#0f172a',
  fg2:     '#334155',
  fg3:     '#64748b',
  fg4:     '#94a3b8',
  line:    'rgba(15, 23, 42, 0.08)',
  axis:    'rgba(15, 23, 42, 0.12)',
  primary: '#4f46e5',
  cyan:    '#0ea5e9',
  pos:     '#059669',
  neg:     '#dc2626',
  cau:     '#d97706',
};

function applyDefaults() {
  if (window.__chartDefaults) return;
  window.__chartDefaults = true;
  Chart.defaults.color = CHART.fg3;
  Chart.defaults.borderColor = CHART.line;
  Chart.defaults.font.family = "'Inter', system-ui, sans-serif";
  Chart.defaults.font.size = 11.5;
  Chart.defaults.plugins.legend.labels.color = CHART.fg2;
  Chart.defaults.plugins.legend.labels.boxWidth = 8;
  Chart.defaults.plugins.legend.labels.boxHeight = 8;
  Chart.defaults.plugins.legend.labels.usePointStyle = true;
  Chart.defaults.plugins.legend.labels.padding = 14;
  Chart.defaults.plugins.tooltip.backgroundColor = '#ffffff';
  Chart.defaults.plugins.tooltip.titleColor = CHART.fg;
  Chart.defaults.plugins.tooltip.bodyColor  = CHART.fg2;
  Chart.defaults.plugins.tooltip.borderColor = CHART.axis;
  Chart.defaults.plugins.tooltip.borderWidth = 1;
  Chart.defaults.plugins.tooltip.padding = 10;
  Chart.defaults.plugins.tooltip.cornerRadius = 8;
  Chart.defaults.plugins.tooltip.titleFont = { weight: '600' };
  Chart.defaults.plugins.tooltip.bodyFont  = { family: "'JetBrains Mono', ui-monospace, monospace", size: 11 };
}

function commonScales(yMax = 100) {
  return {
    x: {
      grid: { display: false },
      ticks: { color: CHART.fg3 },
      border: { display: false },
    },
    y: {
      beginAtZero: true,
      max: yMax,
      grid: { color: CHART.line, drawBorder: false },
      ticks: { color: CHART.fg3, stepSize: 25 },
      border: { display: false },
    },
  };
}

function makeBarChart(canvas, labels, values, opts = {}) {
  applyDefaults();
  const color = opts.color || CHART.primary;
  return new Chart(canvas, {
    type: 'bar',
    data: {
      labels,
      datasets: [{
        data: values,
        backgroundColor: color + '88',
        borderColor: color,
        borderWidth: 0,
        borderRadius: 6,
        barThickness: 24,
        maxBarThickness: 28,
      }],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: { legend: { display: false } },
      scales: commonScales(opts.yMax || 100),
    },
  });
}

function makePairedBarChart(canvas, labels, datasets) {
  applyDefaults();
  const palette = [CHART.primary, CHART.cyan];
  return new Chart(canvas, {
    type: 'bar',
    data: {
      labels,
      datasets: datasets.map((d, i) => ({
        ...d,
        backgroundColor: palette[i % palette.length] + 'aa',
        borderColor:     palette[i % palette.length],
        borderWidth: 0,
        borderRadius: 6,
        barThickness: 18,
      })),
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: { legend: { position: 'bottom', align: 'start' } },
      scales: commonScales(),
    },
  });
}

function makeLineChart(canvas, datasets, options = {}) {
  applyDefaults();
  return new Chart(canvas, {
    type: 'line',
    data: { datasets },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      interaction: { mode: 'index', intersect: false },
      plugins: { legend: { position: 'bottom', align: 'start' } },
      scales: {
        x: {
          type: 'linear',
          min: options.xMin || 1,
          max: options.xMax || 5,
          grid: { color: CHART.line },
          ticks: { color: CHART.fg3, stepSize: 1 },
          border: { display: false },
          title: options.xTitle ? { display: true, text: options.xTitle, color: CHART.fg3 } : undefined,
        },
        y: {
          beginAtZero: true,
          max: 100,
          grid: { color: CHART.line },
          ticks: { color: CHART.fg3, stepSize: 25 },
          border: { display: false },
          title: options.yTitle ? { display: true, text: options.yTitle, color: CHART.fg3 } : undefined,
        },
      },
    },
  });
}

function makeRadarChart(canvas, labels, values, color = CHART.primary) {
  applyDefaults();
  return new Chart(canvas, {
    type: 'radar',
    data: {
      labels,
      datasets: [{
        data: values,
        backgroundColor: color + '20',
        borderColor: color,
        borderWidth: 1.6,
        pointBackgroundColor: color,
        pointBorderColor: '#fff',
        pointBorderWidth: 1.4,
        pointRadius: 4,
        pointHoverRadius: 6,
      }],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      plugins: { legend: { display: false } },
      scales: {
        r: {
          angleLines: { color: CHART.axis },
          grid:       { color: CHART.line, circular: true },
          ticks:      { display: false },
          pointLabels:{ color: CHART.fg2, font: { size: 11 } },
          suggestedMin: 0, suggestedMax: 100,
        },
      },
    },
  });
}

function makeDoughnut(canvas, labels, values, colors) {
  applyDefaults();
  return new Chart(canvas, {
    type: 'doughnut',
    data: {
      labels,
      datasets: [{
        data: values,
        backgroundColor: colors,
        borderColor: '#fff',
        borderWidth: 3,
      }],
    },
    options: {
      responsive: true, maintainAspectRatio: false,
      cutout: '74%',
      plugins: { legend: { position: 'bottom', align: 'start' } },
    },
  });
}
