# EVA Guardian — MVP Plan of Action

Chronological build-out plan for the EVA Guardian localized alert system MVP.
See [Moon2Mars_Individual.md](Moon2Mars_Individual.md) for the proposal this supports.

## Status

Scaffold complete and verified building/running via terminal (`cmake` + MSVC, no
Visual Studio IDE required):
- Async in-process pub/sub `MessageBus` (worker thread + queue)
- `SuitTelemetryFrame` (combined o2/pressure/thermal per suit per tick) / `Alert` types
- `O2Monitor`, `PressureMonitor`, `ThermalMonitor` components with Warning/Critical
  tiers and edge-triggered alerting (fire only on severity change, not every tick)
- `Simulator` + stdin command loop in `main.cpp` for live demo input

## Steps

### 1. Live/scriptable simulator — DONE
`Simulator` publishes one combined `SuitTelemetryFrame` (o2 + pressure + thermal
together, matching how a real suit reports housekeeping telemetry per sample
interval) on the `"telemetry.suit"` topic every second on a background thread.
`main.cpp` reads stdin commands (`o2 <value>`, `pressure <value>`,
`thermal <value>`, `reset`, `quit`) and forwards them to `simulator.handleCommand()`
to override values live during a demo. All three monitors subscribe to the
same topic and each reads only the field it cares about.

### 2. Two-tier thresholds (Warning vs Critical) — DONE
Each monitor now classifies each frame as Info (nominal) / Warning / Critical
and tracks its last published level, only publishing an alert when the level
changes. This also fixed a real bug: alerts previously fired on every tick
while a value stayed out of range, flooding the same console used for stdin
commands and making it impossible to type. Recovery back to nominal now emits
a one-time `[INFO]` alert instead of just going silent.

### 3. Multi-suit decision support + live dashboard — DONE
Threshold checking alone reads as "basic math" in a demo — this step is what
makes the system feel like an actual decision-support product:
- **Multi-suit**: run 3 independent simulated suits at once (`suit-1`,
  `suit-2`, `suit-3`), each with its own `Simulator` instance publishing on
  the shared `"telemetry.suit"` topic. Commands are now `<suit> <metric>
  <value>` / `<suit> reset` so you target a specific suit. Monitors track
  edge-triggered state **per suit ID** (`unordered_map<suitId, AlertSeverity>`)
  instead of a single `lastLevel_`, so suits don't clobber each other's state.
- **Recommended actions**: each `Alert` now carries a `metric` and
  `recommendedAction` string tied to its severity (e.g. critical O2 → "Abort
  EVA immediately: return to airlock and switch to backup O2 supply."),
  directly reflecting the proposal's "decision support" pitch rather than
  just flagging out-of-range values.
- **Live dashboard**: a `Dashboard` class subscribes to both `"telemetry.suit"`
  and `"alerts"`, tracks latest status per suit/metric, and renders an
  on-demand console table via the `dashboard` command — stand-in for the
  "visual dashboard inside the lunar habitat" from the proposal. In a real deployment a habitat likely
  already has its own dashboard software; the point is that the pub/sub bus
  is the integration surface — any subscriber (ours or an existing one) can
  plug into the same `"alerts"` topic without the engine owning the UI. Our
  console table is just a demo stand-in to prove that out.

### 4. Earth-independence demo (simulated Mission Control relay) — DONE
Added `EarthRelay`, a subscriber that reacts to the same `"alerts"` topic but
only after a simulated 8s light-delay round trip (its own internal
queue + worker thread, so the delay never blocks the bus or other
subscribers). `main.cpp`'s direct alert printer is now labeled `[LOCAL, ...]`
and prints instantly, while `EarthRelay` prints the same alert labeled
`[EARTH RELAY, +8000ms delay]` several seconds later — a side-by-side, visual
demonstration of the proposal's core pitch (Earth-independent, immediate
decision support) instead of just asserting it in prose.

### 5. Composite/correlated risk reasoning — DONE
Added `RiskAssessor`, a subscriber that tracks each suit's latest O2/Pressure/
Thermal severity (from the same `"alerts"` topic) and escalates to a
`Critical` composite alert once 2+ metrics are simultaneously abnormal for
the same suit (e.g. "Multiple simultaneous anomalies detected (O2, Pressure)
-> Recommend full EVA abort..."), de-escalating with an `[INFO]` recovery
alert once fewer than 2 remain abnormal. Edge-triggered per suit like the
other monitors. This demonstrates why the pub/sub architecture matters:
`RiskAssessor` composes signals from independently-built monitors into a
diagnosis none of them could raise alone.

### 6. Predictive "time to critical" alerts — DONE
Added `TrendPredictor`, a subscriber that tracks each suit's previous sample
(value + timestamp) per metric, computes rate of change, and projects
forward to estimate time until the critical threshold is crossed. If that
projection is within a 60s horizon, it publishes a one-time `[WARNING]`
alert (e.g. "Projected to reach critical O2 in ~4s at current rate ->
Prepare corrective action now..."), distinct from and typically arriving
before the reactive monitors' own Warning/Critical alerts. Also extracted
`eva/thresholds.hpp` with the shared safety threshold constants so the
reactive monitors and this predictive component can't drift out of sync.
`Simulator` gained a `<suit> <metric> drift <rate/s>` command so a value can
trend gradually instead of only jumping instantly, which is what makes this
feature demoable.

### 7. Post-EVA debrief report — DONE
Added `DebriefRecorder`, a subscriber that tracks per-suit min/max for each
metric, time spent in each severity state (Info/Warning/Critical), reactive
alert counts, predictive-warning counts, and composite-escalation counts.
`quit` calls `printReport()` before exiting, printing a per-suit summary
table — a tangible end-of-session artifact for a demo.

### 8. Alert/telemetry logging polish — DONE
Extracted `eva/log_format.hpp` (`currentTimestamp()`, `severityToString()`,
`formatAlert()`) so every place an alert gets printed shares one consistent,
timestamped format: `[HH:MM:SS.mmm] [label, SEVERITY] source (suitId):
message -> action`. `main.cpp`'s LOCAL printer and `EarthRelay` both use it
now instead of each having their own ad hoc formatting. Added `AlertLogger`,
a subscriber that writes every alert to `eva_guardian.log` (truncated each
run) in the same format, giving a durable demo artifact.

### 9. Basic tests (CTest) — DONE
Refactored the build into an `eva_core` static library (everything except
`main.cpp`) linked by both `eva_guardian` and a new `eva_tests` executable,
registered with CTest via `enable_testing()`/`add_test()`. Added a minimal
self-registering `TEST_CASE`/`CHECK` harness (no external framework
dependency) in `tests/`, covering:
- `MessageBus` delivers published messages only to matching-topic subscribers
- `O2Monitor` stays silent when nominal and fires at the critical boundary
- Per-suit edge-triggered state doesn't cross-contaminate between suits
- `RiskAssessor` escalates only once 2+ metrics are abnormal for the same suit

Run via `ctest -C Debug --output-on-failure` from the `build/` directory
(all 6 checks pass).

### 10. README + demo script — DONE
Added [demo.ps1](demo.ps1): a scripted, timed walkthrough that launches
`eva_guardian.exe` via `System.Diagnostics.Process` (stdin redirected only,
stdout left attached to the console so it displays live) and sends commands
with real `Start-Sleep` pacing between them. One run touches every feature:
baseline dashboard -> gradual O2 drift (predictive alert fires before the
reactive one) -> Earth-relay contrast (~8s delayed echo) -> second
simultaneous anomaly -> composite risk escalation -> dashboard -> recovery
-> independent multi-suit anomalies -> final dashboard -> quit -> post-EVA
debrief. Verified end-to-end. Run with
`powershell -ExecutionPolicy Bypass -File .\demo.ps1` (also added
`std::cout.setf(std::ios_base::unitbuf)` in `main()` so output isn't fully
buffered when redirected). Added [README.md](README.md) with a product
description, current feature list, build/run/test instructions, and a
step-by-step "what to expect" walkthrough of the demo.

### 11. Final polish pass
Clean up TODOs, verify a clean build from scratch, rehearse the demo end-to-end.
