import { useMemo, useState } from 'react';
import { BondForm } from './components/BondForm';
import { CashflowChart } from './components/CashflowChart';
import { MetricsPanel } from './components/MetricsPanel';
import { PriceYieldChart } from './components/PriceYieldChart';
import { ScenarioPanel } from './components/ScenarioPanel';
import { YieldCurvePanel } from './components/YieldCurvePanel';
import { useEngine } from './engine/useEngine';
import { DEFAULTS, type BondInputs } from './model';

export default function App() {
  const { engine, error } = useEngine();
  const [inputs, setInputs] = useState<BondInputs>(DEFAULTS);
  const onChange = (patch: Partial<BondInputs>) =>
    setInputs((prev) => ({ ...prev, ...patch }));

  const computed = useMemo(() => {
    if (!engine) return null;
    const { face, couponPct, ytmPct, years, freq, dayCount } = inputs;
    const numbers = [face, couponPct, ytmPct, years];
    if (!numbers.every(Number.isFinite) || years < 1) return null;
    try {
      const analytics = engine.analyze(face, couponPct, freq, years, ytmPct, dayCount);
      const lo = Math.max(0.25, ytmPct - 5);
      const hi = ytmPct + 5;
      const priceYield = engine.priceYieldCurve(face, couponPct, freq, years, lo, hi, 60, dayCount);
      const cashflows = engine.cashflows(face, couponPct, freq, years, dayCount);
      return { analytics, priceYield, cashflows };
    } catch {
      return null;
    }
  }, [engine, inputs]);

  return (
    <div className="app">
      <header className="app-header">
        <div>
          <h1>Fixed-Income Analytics</h1>
          <p className="subtitle">
            Bond pricing, yield, duration &amp; convexity, from a C++ engine compiled to WebAssembly.
          </p>
        </div>
        <a
          className="repo-link"
          href="https://github.com/M47H13UK/cpp-fixed-income-analytics"
          target="_blank" rel="noreferrer"
        >
          View source ↗
        </a>
      </header>

      {error && (
        <div className="banner error">
          Failed to load the WASM engine: {error}. Run <code>npm run build:wasm</code> first.
        </div>
      )}

      <main className="layout">
        <BondForm inputs={inputs} onChange={onChange} />

        <div className="results">
          {!engine && !error && <div className="panel loading">Loading engine…</div>}
          {computed && engine && (
            <>
              <MetricsPanel analytics={computed.analytics} face={inputs.face} />
              <ScenarioPanel engine={engine} inputs={inputs} base={computed.analytics} />
              <div className="charts">
                <PriceYieldChart
                  data={computed.priceYield}
                  currentYield={inputs.ytmPct}
                  currentPrice={computed.analytics.clean}
                />
                <CashflowChart data={computed.cashflows} />
              </div>
              <YieldCurvePanel engine={engine} />
            </>
          )}
        </div>
      </main>

      <footer className="app-footer">
        C++20 · Emscripten / WebAssembly · React + TypeScript · Recharts
      </footer>
    </div>
  );
}
