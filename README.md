<div align="center">

# 📚 studyhub

### Smart Student Academic & Alert System

**A pure C++14 academic-intelligence backend with a modern HTML / CSS / JS dashboard.**
Built as a deep showcase of object-oriented design — **inheritance, polymorphism, templates,
smart pointers, custom exception hierarchies, and five GoF patterns** (Strategy, Observer,
Singleton, Factory, Repository) — wired together through a hand-written Winsock HTTP server,
with **no third-party C++ runtime dependencies** beyond a single-header JSON library.

[![C++14](https://img.shields.io/badge/C++-14-00599C?logo=cplusplus&logoColor=white)]()
[![Backend](https://img.shields.io/badge/backend-pure_C++-blue)]()
[![No deps](https://img.shields.io/badge/runtime_deps-zero-success)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()
[![Status](https://img.shields.io/badge/status-active-brightgreen)]()
[![Patterns](https://img.shields.io/badge/GoF_patterns-5-blueviolet)]()

</div>

---

## ✨ What is this?

`studyhub` is a self-contained student dashboard. One executable serves both a JSON REST API
and the static frontend — open the page, get a live picture of attendance, marks, exam
countdowns, study plan, **Bayesian risk score**, **OLS-regression grade prediction**, **burnout
detector** and seven other analyzers. Everything is **fully under the student's control** —
add / edit / delete subjects, marks, exams, events, competitions and goals.

```
                ╭─────────────────────────────────────────╮
                │  Browser  ←  fetch JSON  ←  ssaas_server │
                │           (HTML/CSS/JS)     (C++14, 1 MB)│
                ╰─────────────────────────────────────────╯
```

---

## 🎯 Highlights

|     | Feature | What it does |
|-----|---------|---|
| 🎓 | **Academic Health Score** | Weighted blend of attendance + credit-weighted performance, with status banding. |
| 📅 | **Safe Bunk Predictor** | Solves `attended ÷ (total + b) ≥ 75 %` per subject — exactly how many lectures you can still skip. |
| ⚠️ | **Attendance Risk** | Per-subject Safe / Warning / Critical, with a 10-lecture trailing window for *trending* dips. |
| 📉 | **Performance Drop Detection** | Flags subjects where the latest score fell more than 8 pts below the historical mean. |
| ⚡ | **Academic Pressure Meter** | Composite of attendance gap, performance gap and 14-day exam load. |
| ⏰ | **Exam Countdown** | Live days-until calculator with urgency banding and prep tracking. |
| 🧠 | **Smart Study Plan** | Sorts subjects by `weakness × proximity × attendance drag` → Morning / Afternoon / Evening blocks. |
| 📈 | **Grade Predictor (OLS)** | Linear regression on chronological assessments — slope, intercept, R², CI band, required-in-finals. |
| 🎲 | **Bayesian Risk Score** | Naive-Bayes log-odds aggregation over 5 academic indicators. |
| 🩺 | **Burnout Detector** | Multi-factor wellness score: attendance velocity + variance jump + 14-day exam load. |
| 🎟️ | **Events & Competitions** | Track talks, hackathons, contests, deadlines — with full register / unregister flow. |
| 🎯 | **Goals** | CGPA / attendance / study-hours / marks targets with progress bars. |
| 📆 | **Unified Schedule** | Calendar widget combining exams, events and competition deadlines. |

---

## 🖼️ Visual tour

Add screenshots here once you build it locally — drop PNGs into `docs/screenshots/` and they'll
render in the *Frontend pages* table further down. The dashboard is built with three font
files (Inter + JetBrains Mono) and a single CSS file, so it renders identically across
browsers.

| Theme | Light · off-white background `#f5f7fb` |
|-------|----------------------------------------|
| Sidebar | Deep indigo `#1e1b4b` with white nav |
| Accent | Single indigo `#4f46e5` — used semantically |
| Cards | White, soft 1-2 px shadows, hover lift |
| Type | Inter (400 / 500 / 600 / 700) + JetBrains Mono for data |
| Data viz | Chart.js with monochromatic palette + per-code accent on multi-series |

Across **eleven pages**, every domain object exposes full CRUD via 3-dot action menus, modal
dialogs with proper validation, confirmation prompts before destructive operations, and live
toasts for feedback.

---

## 🏗️ Architecture

```
┌────────────────────────────────────────────────────────────────────┐
│   Browser (Chart.js + vanilla JS)                                  │
│   index · subjects · attendance · marks · exams · schedule         │
│   events · competitions · goals · analytics · profile              │
└──────────────────────────────┬─────────────────────────────────────┘
                               │  fetch() — JSON over HTTP/1.1
┌──────────────────────────────▼─────────────────────────────────────┐
│   ssaas_server.exe   (single-process, single-threaded, ~1 MB)      │
│  ┌────────────────────────────────────────────────────────────┐   │
│  │ HttpServer  ←  Router  ←  Controllers   (REST endpoints)   │   │
│  └─────────────────────────────────┬──────────────────────────┘   │
│  ┌──────────────────────┐  ┌───────▼────────────┐  ┌───────────┐ │
│  │ AlertSystem (Obs.)   │  │  IAnalyzer ×10      │  │ DataStore │ │
│  │  IAlert hierarchy    │  │  (Strategy)         │  │ Singleton │ │
│  │  AlertFactory        │  │  Health · Bunk · …  │  │ Repo<T>   │ │
│  └──────────────────────┘  └─────────────────────┘  └───────────┘ │
│                                       Domain: Person → Student     │
│                                       owns Subject · Marks ·       │
│                                       Attendance · Exam · Event ·  │
│                                       Competition · Goal           │
└────────────────────────────────────────────────────────────────────┘
```

* **Backend:** C++14, MinGW-friendly. Single executable serves both REST APIs and the static frontend.
* **HTTP:** Hand-written Winsock2 server — no cpp-httplib, no Boost, no asio.
* **JSON:** [`nlohmann/json`](https://github.com/nlohmann/json) — single header.
* **Frontend:** Static HTML / CSS, vanilla JS, [Chart.js](https://chartjs.org) loaded from CDN.

---

## 🚀 Quick start

### Requirements

* Windows + MinGW (developed against `g++ 6.3.0`; any `g++ ≥ 5` with C++14 support works).
* Anything that can compile against Winsock2 (`-lws2_32 -lwsock32`).

### Build (Windows)

```bat
cd backend
build.bat
```

Produces `backend\build\ssaas_server.exe` (~1 MB).

### Build (any UNIX-y shell with `make`)

```bash
cd backend
make            # → build/ssaas_server.exe
make smoke      # console smoke test of every analyzer
make clean
```

### Run

```bat
cd backend\build
ssaas_server.exe --host 127.0.0.1 --port 8090 ^
                 --data ..\data\sample_data.json ^
                 --static ..\..\frontend
```

Open **http://127.0.0.1:8090** in any modern browser.

| Flag | Default | Purpose |
|------|---------|---------|
| `--host` | `127.0.0.1` | bind address |
| `--port` | `8080` | listen port |
| `--data` | `data/sample_data.json` | seed file |
| `--static` | `../frontend` | static asset root |

---

## 🌐 REST API

### Reads

| Method | Path | Purpose |
|--------|------|---------|
| `GET` | `/api/health` | server liveness |
| `GET` | `/api/student` | active student summary |
| `GET` | `/api/subjects` | enrolled subjects + per-subject metrics |
| `GET` | `/api/marks` | every marks record (with `id` index) |
| `GET` | `/api/exams` | every scheduled exam |
| `GET` | `/api/events` | every event |
| `GET` | `/api/competitions` | every competition |
| `GET` | `/api/goals` | every goal |
| `GET` | `/api/alerts` | recomputes & returns all polymorphic alerts |
| `GET` | `/api/dashboard` | one-shot bundle: student + every analyzer + alerts |
| `GET` | `/api/analytics/health` | health score |
| `GET` | `/api/analytics/bunk` | safe-bunk + recovery plan |
| `GET` | `/api/analytics/risk` | per-subject risk classification |
| `GET` | `/api/analytics/performance` | drop detection |
| `GET` | `/api/analytics/pressure` | academic pressure meter |
| `GET` | `/api/analytics/predict` | OLS grade prediction + R² + CI |
| `GET` | `/api/analytics/bayes` | Bayesian risk + indicators |
| `GET` | `/api/analytics/burnout` | burnout detector |
| `GET` | `/api/analytics/exams` | exam countdown |
| `GET` | `/api/analytics/study-plan` | smart study plan |

### Writes (full CRUD)

| Resource | Create | Read | Update | Delete |
|----------|--------|------|--------|--------|
| **Student** | _seeded_ | `GET /api/student` | `PUT /api/student` | _n/a_ |
| **Subjects** | `POST /api/subjects` | `GET /api/subjects` | `PUT /api/subjects/:code` | `DELETE /api/subjects/:code` |
| **Marks** | `POST /api/marks` | `GET /api/marks` | `PUT /api/marks/:id` | `DELETE /api/marks/:id` |
| **Exams** | `POST /api/exams` | `GET /api/exams` | `PUT /api/exams/:id` | `DELETE /api/exams/:id` |
| **Attendance** | `POST /api/attendance` | (via subjects) | `PUT /api/attendance/:code` | _n/a_ |
| **Events** | `POST /api/events` | `GET /api/events` | `PUT /api/events/:id` | `DELETE /api/events/:id` |
| **Competitions** | `POST /api/competitions` | `GET /api/competitions` | `PUT /api/competitions/:id` | `DELETE /api/competitions/:id` |
| **Goals** | `POST /api/goals` | `GET /api/goals` | `PUT /api/goals/:id` | `DELETE /api/goals/:id` |

CORS is enabled (`Access-Control-Allow-Origin: *`) so the frontend can be served from `file://` during development.

---

## 🎨 Object-Oriented Design

### Inheritance & Polymorphism

```
            Person  (abstract — pure virtual getRole)
              ▲
              └──── Student   (introduces program, semester,
                               owns subjects/marks/…/goals)

            IAlert  (abstract — virtual getMessage / getActionHint / toJson)
              ▲
              ├──── AttendanceAlert      (per-subject, threshold-based)
              ├──── PerformanceAlert     (drop-delta, historical avg)
              ├──── ExamAlert            (days-until, prep%)
              └──── BurnoutAlert         (composite wellness score)

            IAnalyzer  (abstract — Strategy interface)
              ▲
              ├──── HealthScoreAnalyzer
              ├──── BunkPredictor
              ├──── AttendanceRiskAnalyzer
              ├──── PerformanceAnalyzer
              ├──── PressureAnalyzer
              ├──── GradePredictor       (linear regression)
              ├──── BayesianRiskScorer   (naive Bayes)
              ├──── BurnoutDetector      (multi-factor)
              ├──── ExamCountdownAnalyzer
              └──── StudyPlanAnalyzer
```

### Design patterns demonstrated

| Pattern | Where |
|---------|-------|
| **Strategy**   | `IAnalyzer` + 10 concrete analyzers, plugged in `Controllers::registerAll`. |
| **Observer**   | `AlertSystem::subscribe(listener)` → recomputes & pushes JSON snapshots whenever data changes. |
| **Singleton**  | `Logger`, `DataStore`, `AlertSystem` (`getInstance()` with deleted copy ops). |
| **Factory**    | `AlertFactory::deriveAlerts(student)` materialises a heterogeneous `vector<unique_ptr<IAlert>>`. |
| **Repository** | `Repository<T>` template generic key-based store; specialised here as `Repository<Student>`. |
| **Facade**     | `Controllers` collapses analyzer + storage + alert calls into clean REST endpoints. |

### OOP showcases

| Concept | Where in the code |
|---------|---|
| Encapsulation | All domain classes expose only validated setters/getters. |
| Operator overloading | `Date` defines `<`, `==`, `<=`, …, `operator-`, and `operator<<`. |
| Templates | `Repository<T>` is fully generic with a custom `KeyExtractor` functor. |
| Smart pointers | `std::unique_ptr<IAlert>` for ownership; `std::shared_ptr<IAnalyzer>` for shared analyzer instances. |
| STL | `vector`, `unordered_map`, `algorithm`, `numeric`, `chrono`, lambdas. |
| Exception hierarchy | `SSAASException` → `NotFoundException`, `ValidationException`, `StorageException`, `AnalyticsException`, `HttpException(int code, …)`. |
| RAII | Winsock initialisation/cleanup tied to `HttpServer` ctor/dtor. |

---

## 🔬 Algorithms & models

### Safe Bunk Predictor

Given `a` lectures attended, `t` total, target ratio `T`:

* If currently safe (`a / t ≥ T`):
  ```
  b_max = ⌊ a / T  −  t ⌋    ← max future absences keeping ratio ≥ T
  ```
* If currently below target:
  ```
  n_min = ⌈ (T·t − a) / (1 − T) ⌉   ← consecutive presents to recover
  ```

### OLS Grade Predictor

Standard ordinary least-squares fit on `(x, y)` pairs (`x` = assessment index, `y` = score%):

```
β1 = Σ(xi − x̄)(yi − ȳ) / Σ(xi − x̄)²
β0 = ȳ − β1·x̄
```

Also exports R², residual standard deviation (used for a 95 % CI band), the predicted next
score, and the score required in finals to hit the target average.

### Bayesian Risk Scorer

Indicators are independent Bernoulli signals. Each contributes a likelihood ratio
`LR_i = P(E_i | risk) / P(E_i | safe)`, scaled by an evidence-strength function
`LR_i = max_ratio ^ strength`. Combined in log-odds space:

```
log_posterior_odds = log_prior_odds + Σ (active i) log LR_i
posterior          = σ(log_posterior_odds)
```

Bands: `< 0.25` Low · `< 0.45` Moderate · `< 0.70` High · else Critical.

### Burnout Detector

```
attendance_velocity = (attendance % in second half of records) − (first half)
variance_jump       = var(later marks) − var(earlier marks)
exam_load           = upcoming exams within 14 days

score = 0.45·norm(velocity, 30) + 0.30·norm(variance_jump, 200) + 0.25·norm(exam_load, 4)
```

Bands: `< 30` Calm · `< 60` Watch · else Burnout Risk.

---

## 📁 Project layout

```
studyhub/
├── README.md                      ← you are here
├── .claude/launch.json
├── .gitignore
│
├── backend/
│   ├── build.bat                  ← Windows build script
│   ├── Makefile                   ← cross-platform build
│   ├── main.cpp                   ← CLI bootstrap
│   ├── data/
│   │   └── sample_data.json       ← seed student + subjects + marks + exams + events
│   ├── include/
│   │   ├── core/                  ← Person, Student, Subject, Date, Event, Competition, Goal
│   │   ├── analytics/             ← IAnalyzer + 10 strategies
│   │   ├── alerts/                ← IAlert hierarchy + Factory + Observer
│   │   ├── storage/               ← Repository<T>, DataStore
│   │   ├── server/                ← Http*, Router, Controllers
│   │   ├── utils/                 ← Logger, Exceptions
│   │   └── third_party/json.hpp   ← single-header nlohmann/json
│   └── src/                       ← .cpp implementations mirroring include/
│
└── frontend/
    ├── index.html                 ← Overview dashboard
    ├── subjects.html              ← Catalogue with full CRUD
    ├── attendance.html            ← Risk + safe-bunk + quick mark
    ├── marks.html                 ← OLS prediction chart + add/edit
    ├── exams.html                 ← Countdown + prep slider
    ├── schedule.html              ← Unified calendar widget
    ├── events.html                ← Events with type filter + register
    ├── competitions.html          ← Competitions with status filter
    ├── goals.html                 ← Targets + progress bars
    ├── analytics.html             ← Radar + Bayes + burnout
    ├── profile.html               ← Student profile + edit
    ├── css/style.css
    └── js/{api.js, app.js, charts.js}
```

---

## 📐 Frontend pages

| Page | Highlights |
|------|------------|
| **Overview** | Hero welcome card · 4 stat tiles · SVG health gauge · Bayesian indicator strip · smart study plan · live alerts side panel · upcoming exams · goals · subject snapshot table. |
| **Subjects** | Card grid with attendance + average bars · 3-dot action menu (edit / set attendance / delete) · cascading-delete confirmation. |
| **Attendance** | Risk doughnut · safe-bunk table with quick Present / Absent buttons · paired bar chart of recent vs cumulative %. |
| **Marks** | Multi-line OLS regression with dotted predictive segments · drop-detection table with Δ / R² / required-in-finals · all-assessments table with edit/delete. |
| **Exams** | Big countdown to nearest exam · full schedule with urgency-coloured days · per-exam prep slider · add/edit/delete. |
| **Schedule** | Full calendar widget · prev/next-month navigation · today highlighted · per-day list · 7-day side panel · legend. |
| **Events** | Type-filtered grid · register/unregister toggle · add/edit/delete · academic / cultural / sports / career / workshop. |
| **Competitions** | Status-filtered grid · prize highlight · Mark won / registered / missed shortcuts · full CRUD. |
| **Goals** | CGPA · Attendance · Study hours · Marks · Custom · per-goal progress card with quick-update modal. |
| **Analytics** | 9-axis multi-factor risk radar · Bayesian indicator table with effect-on-posterior · burnout component bar chart · study plan repeat. |
| **Profile** | Indigo profile hero · details card · live snapshot tiles · edit profile modal. |

---

## 💡 Why this exists

Built as a deep OOP showcase for an academic course. It is intentionally small enough to read
end-to-end yet large enough to demonstrate production-grade separation of concerns: a domain
model, a strategy-based analytics layer, a polymorphic alert system, a hand-written HTTP layer,
and a purely declarative frontend.

The full pipeline is pure C++14 standard library plus a single-header JSON library — no Boost,
no asio, no cpp-httplib, no threads. **You can read every line of code that handles your data.**

---

## 📜 License

MIT — feel free to fork, extend, or take it apart for learning.

---

<div align="center">

*Made with C++ pointers &amp; a lot of chai. ☕*

</div>
