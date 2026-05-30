import { useState } from 'react';
import {
  CartesianGrid, Legend, Line, LineChart, ResponsiveContainer, Tooltip, XAxis, YAxis,
} from 'recharts';
import type { Engine } from '../engine/types';
import { fmt } from '../model';

// Upward-sloping par yields for 1y..7y, used as the editable default.
const DEFAULT_PAR = [4.0, 4.4, 4.7, 4.9, 5.0, 5.05, 5.1];

const tooltipStyle = {
  background: '#0d1320',
  border: '1px solid #243044',
  borderRadius: 8,
  fontSize: 12,
};

export function YieldCurvePanel({ engine }: { engine: Engine }) {
  const [par, setPar] = useState<number[]>(DEFAULT_PAR);
  const setOne = (i: number, v: number) =>
    setPar((prev) => prev.map((x, idx) => (idx === i ? v : x)));

  let data: ReturnType<Engine['bootstrapCurve']> = [];
  if (par.every(Number.isFinite)) {
    try { data = engine.bootstrapCurve(par); } catch { data = []; }
  }

  return (
    <section className="panel">
      <h2 className="panel-title">Yield curve · bootstrapped zero &amp; forward rates</h2>
      <p className="chart-caption">
        Enter par (coupon) yields for maturities from 1 to {par.length}y; the engine bootstraps
        the zero curve and the implied 1-year forward rates.
      </p>

      <div className="par-inputs">
        {par.map((v, i) => (
          <label key={i} className="par-input">
            <span>{i + 1}y</span>
            <input
              type="number" step={0.1}
              value={Number.isFinite(v) ? v : ''}
              onChange={(e) => setOne(i, parseFloat(e.target.value))}
            />
          </label>
        ))}
      </div>

      <ResponsiveContainer width="100%" height={280}>
        <LineChart data={data} margin={{ top: 8, right: 20, bottom: 8, left: 4 }}>
          <CartesianGrid stroke="#1b2533" strokeDasharray="3 3" />
          <XAxis
            dataKey="year" tickFormatter={(v: number) => `${v}y`}
            stroke="#6b7a90" fontSize={12} tickMargin={8}
          />
          <YAxis
            tickFormatter={(v: number) => `${v.toFixed(1)}%`}
            stroke="#6b7a90" fontSize={12} width={52}
          />
          <Tooltip
            contentStyle={tooltipStyle}
            labelFormatter={(v: number) => `${fmt(v, 0)}y`}
            formatter={(value: any, name: any) => [`${fmt(value)}%`, name]}
          />
          <Legend wrapperStyle={{ fontSize: 12 }} />
          <Line type="monotone" dataKey="par" name="Par" stroke="#6b7a90"
                strokeWidth={1.6} strokeDasharray="4 4" dot={false} />
          <Line type="monotone" dataKey="zero" name="Zero" stroke="#5b8cff"
                strokeWidth={2.2} dot={false} />
          <Line type="monotone" dataKey="forward" name="Forward (1y)" stroke="#2dd4a7"
                strokeWidth={2.2} dot={false} />
        </LineChart>
      </ResponsiveContainer>
    </section>
  );
}
