import { useState } from 'react';
import type { Analytics, Engine } from '../engine/types';
import type { BondInputs } from '../model';
import { fmt } from '../model';

interface Props {
  engine: Engine;
  inputs: BondInputs;
  base: Analytics;
}

// Compares the duration+convexity approximation of a yield move against a full
// reprice - the classic check of how far the second-order risk model holds.
export function ScenarioPanel({ engine, inputs, base }: Props) {
  const [bp, setBp] = useState(50);
  const { face, couponPct, freq, years, ytmPct, dayCount } = inputs;

  const dy = bp / 10000;                       // basis points -> decimal
  const shifted = engine.analyze(face, couponPct, freq, years, ytmPct + bp / 100, dayCount);
  const actual = shifted.dirty - base.dirty;
  const estimate = base.dirty * (-base.modified * dy + 0.5 * base.convexity * dy * dy);
  const errorBp = actual !== 0 ? ((estimate - actual) / Math.abs(actual)) * 100 : 0;

  return (
    <section className="panel">
      <h2 className="panel-title">Scenario · parallel yield shift</h2>
      <p className="chart-caption">
        Shift the yield and compare the duration + convexity estimate with a full
        reprice. They agree for small moves and diverge as the move grows.
      </p>

      <div className="scenario-control">
        <input
          type="range" min={-250} max={250} step={5} value={bp}
          onChange={(e) => setBp(Number(e.target.value))}
        />
        <span className="scenario-bp">{bp > 0 ? '+' : ''}{bp} bp</span>
      </div>

      <div className="metric-grid">
        <div className="metric">
          <div className="metric-label">New dirty price</div>
          <div className="metric-value">{fmt(shifted.dirty)}</div>
        </div>
        <div className="metric">
          <div className="metric-label">Actual ΔP (reprice)</div>
          <div className="metric-value">{actual >= 0 ? '+' : ''}{fmt(actual)}</div>
        </div>
        <div className="metric">
          <div className="metric-label">Estimate (dur+convex)</div>
          <div className="metric-value">{estimate >= 0 ? '+' : ''}{fmt(estimate)}</div>
        </div>
        <div className="metric">
          <div className="metric-label">Approx. error</div>
          <div className="metric-value">{fmt(errorBp, 2)}%</div>
        </div>
      </div>
    </section>
  );
}
