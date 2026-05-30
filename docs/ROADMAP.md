# Roadmap: cpp-fixed-income-analytics

> From a first-year homework bond calculator to a proper **fixed-income analytics
> engine** in C++20, compiled to **WebAssembly**, with a **React + TypeScript** web app
> for the charts. The C++ core is the one source of truth; the CLI, tests, and web UI
> are swappable front-ends over it.

**Status legend:** `[ ]` todo · `[~]` in progress · `[x]` done

**Build status (2026-05-30):** Phases 0 to 4 implemented and verified (33 C++ assertions
pass; WASM numerically cross-checked through Node; the web app renders and recomputes
live in a headless-Chrome run, including the bootstrapped yield-curve panel). One depth
item is deferred to the backlog and flagged inline: callable / perpetuity bond **types**.

---

## Locked decisions

| # | Decision | Choice | Rationale |
|---|----------|--------|-----------|
| 1 | Scope | Deep fixed-income toolkit | Depth > breadth; cohesive, C++-native quant signal |
| 2 | Interface | C++ → WASM + React web app | One engine, live clickable demo, charts a TUI can't show |
| 3 | Dates | Lightweight `Date` type, no external dep | Enables settlement / accrued / clean-dirty; keeps WASM small |
| 4 | Repo name | `cpp-fixed-income-analytics` | Keeps `cpp-` prefix to signal language |
| 5 | `main.exe` | Removed going forward, no history rewrite | Avoids force-push; binary just stops being tracked |
| 6 | Frontend | React + TypeScript + Vite + Recharts | Recruiter-familiar, strong charting |
| 7 | Build | CMake + C++20 | Cross-platform, professional |
| 8 | Tests | Catch2 v3 via FetchContent | No system install; assert vs textbook values |
| 9 | Host | GitHub Pages via Actions | Free static hosting for the WASM bundle |

## Target layout

```
cpp-fixed-income-analytics/
├── CMakeLists.txt                  # top-level: engine + cli + tests
├── .gitignore
├── README.md                       # rewritten, live-demo link + screenshots
├── docs/ROADMAP.md                 # this file (living plan)
├── engine/
│   ├── include/fi/
│   │   ├── date.hpp                # lightweight civil date + serial
│   │   ├── daycount.hpp            # 30/360, ACT/360, ACT/365F, ACT/ACT
│   │   ├── bond.hpp                # Bond, Frequency, cashflow schedule
│   │   ├── yield.hpp               # price<->yield, accrued, clean/dirty, YTM solver
│   │   ├── risk.hpp                # Macaulay/modified duration, convexity, DV01
│   │   ├── curve.hpp               # spot/forward/par, bootstrap, price-with-curve
│   │   └── analytics.hpp           # facade returning one Analytics bundle (for bindings)
│   └── src/*.cpp
├── cli/main.cpp                    # thin interactive driver (keeps original demo alive)
├── tests/                          # Catch2 unit tests, golden textbook values
├── wasm/bindings.cpp               # embind: exports the facade to JS
├── web/                            # Vite + React + TS app
└── .github/workflows/ci.yml        # build+test native; build WASM; deploy Pages
```

## Engine conventions

- All rates **decimal** internally (5% → `0.05`); UI converts from percent.
- Prices quoted **per face** unless stated; default face 1000.
- Street/ISMA discounting: exponent `w + i` where `w` = fraction of a coupon period
  to the next coupon, `i` = 0,1,2…; discount factor `1 / (1 + y/f)^(w+i)`.
- Accrued = `coupon * (1 - w)`; clean = dirty − accrued.
- Per-cashflow discounting (not the closed-form annuity) → `y = 0` is well-defined
  (no removable singularity), fixing a latent bug in the original calculator.

## Golden test values (high-confidence anchors)

| Case | Inputs | Expected |
|------|--------|----------|
| Annual coupon | F=1000, c=5%, y=6%, n=5, freq=1, on-coupon | price ≈ **957.876** |
| Semi-annual | F=1000, c=5%, y=6%, n=5, freq=2, on-coupon | price ≈ **957.35** |
| Zero-coupon | F=1000, c=0, y=6%, n=5, freq=1 | price ≈ **747.258** |
| Par | c = y | price ≈ **face** |
| YTM round-trip | price→ytm→price | identity within 1e-9 |
| Duration | analytic vs ±1bp numerical bump | agree within 1e-4 |

---

## Phase 0: Repo hygiene & skeleton  `[x]`
*Independent of all open questions; immediate professionalism signal.*

- [x] Add `.gitignore` (build artifacts, `node_modules`, `dist`, `*.exe`, `.DS_Store`)
- [x] Remove tracked `main.exe`; delete Windows-only `.vscode/tasks.json`
- [x] Top-level `CMakeLists.txt` (C++20, `fi_engine` lib + `cli` exe + `tests` opt-in)
- [x] Move original `main.cpp` → `cli/main.cpp`, rewired onto the library
- [x] `engine` lib so it builds & links (went straight to the full header-only engine)
- [x] Catch2 test asserting the 957.876 anchor; `ctest` green
- [x] Verify: configure, build, run tests locally

## Phase 1: Engine depth (bonds, done properly)  `[x]`
- [x] `date.hpp`: civil date, days-from-civil serial, comparisons
- [x] `daycount.hpp`: 30/360 (US), ACT/360, ACT/365F, ACT/ACT (ISDA); `yearFraction`
- [x] `bond.hpp`: `Bond`, `Frequency`, cashflow `cashflows()` (maturity→settlement)
- [x] `yield.hpp`: `dirtyPrice`, `accruedInterest`, `cleanPrice`,
      `yieldToMaturity` (Newton-Raphson + bisection fallback, handles y≈0)
- [x] `risk.hpp`: `macaulayDuration`, `modifiedDuration`, `convexity`, `dv01`
- [x] Tests: day counts, all golden prices, YTM round-trip, duration vs numerical bump
- [x] Edge cases: zero-coupon, par, y=0, settlement between coupons (accrued ≠ 0)

## Phase 2: Yield curve / term structure  `[x]`
- [x] `curve.hpp`: `YieldCurve` (times + zero rates), `discountFactor`, `zeroRate`,
      `forwardRate`, linear interpolation
- [x] `priceWithCurve`: discount each cashflow at its own spot rate
- [x] `bootstrapFromParYields`: zero curve from par instruments (sequential solve)
- [x] spot ↔ par ↔ forward conversions
- [x] Tests: bootstrap recovers par yields; forward rates consistent with zeros

## Phase 3: WASM bindings + React app  `[x]`
- [x] `analytics.hpp` facade: one call returns {dirty, clean, accrued, ytm,
      macaulay, modified, convexity, dv01, periods} + arrays for charts
- [x] `wasm/bindings.cpp`: embind exports facade + `priceYieldCurve`, `cashflows`, YTM
- [x] Emscripten build (MODULARIZE, EXPORT_ES6) → `engine.mjs` + `engine.wasm`
- [x] Vite + React + TS scaffold; typed `useEngine` loader hook
- [x] `BondForm`, `MetricsPanel`, `PriceYieldChart` (clean price vs yield, current marked)
- [x] Verify: native build, WASM-via-Node cross-check, headless-Chrome render + recompute

## Phase 4: Polish & ship  `[x]` *(2 depth items deferred, see backlog)*
- [x] Charts: cashflow timeline, price/yield curve, parallel-shift P&L (dur+convexity vs reprice)
- [~] Bond types: zero-coupon **done** (coupon = 0); perpetuity / callable **deferred**
- [x] `.github/workflows/ci.yml`: native build+test; emsdk WASM build; deploy to Pages
- [x] Rewrite `README.md`: live link, screenshot, formula appendix, build instructions
- [x] Favicon (bonus polish)
- [x] Rename GitHub repo → `cpp-fixed-income-analytics` (remote done; local folder left to user)
- [x] Yield-curve UI panel: editable par yields → bootstrapped zero & forward chart
- [ ] **Deferred to backlog:** perpetuity / callable bond types (yield-to-worst, OAS)

---

## Open follow-ups (post-MVP)
- Option pricing module (Black-Scholes + Greeks + implied vol) as a second showcase
- Key-rate durations; OAS for callable bonds
- Bloomberg/textbook cross-validation table in the README
